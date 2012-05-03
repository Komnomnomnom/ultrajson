#include <Python.h>
#include <numpy/arrayobject.h>
#include <numpy/halffloat.h>
#include <stdio.h>
#include <datetime.h>
#include <ultrajson.h>

static PyObject* meth_timegm;
static PyObject* mod_calendar;
static PyObject* cls_dataframe;
static PyObject* cls_series;
static PyObject* cls_index;

typedef void *(*PFN_PyTypeToJSON)(JSOBJ obj, JSONTypeContext *ti, void *outValue, size_t *_outLen);


#if (PY_VERSION_HEX < 0x02050000)
typedef ssize_t Py_ssize_t;
#endif

typedef struct __NpyIterContext
{
	PyObject *array;
	NpyIter *iter;
	NpyIter_GetMultiIndexFunc *indexfunc;
	NpyIter_IterNextFunc *iternext;
	char **dataptr;
	npy_intp ndim;
	npy_intp index[NPY_MAXDIMS];
	npy_intp level;
	npy_intp closelevel;
	NPY_ORDER order;

	PyObject* columnLabels;
	npy_intp columnLabelsDim;
	PyObject* rowLabels;
	npy_intp rowLabelsDim;
} NpyIterContext;

typedef struct __TypeContext
{
	JSPFN_ITERBEGIN iterBegin;
	JSPFN_ITEREND iterEnd;
	JSPFN_ITERNEXT iterNext;
	JSPFN_ITERGETNAME iterGetName;
	JSPFN_ITERGETVALUE iterGetValue;
	PFN_PyTypeToJSON PyTypeToJSON;
	PyObject *newObj;
	PyObject *dictObj;
	Py_ssize_t index;
	Py_ssize_t size;
	PyObject *itemValue;
	PyObject *itemName;
	PyObject *attrList;
	char *citemName;

	JSINT64 longValue;

	PyObject* rowLabels;
	PyObject* columnLabels;
	NPY_ORDER npyiterOrder;
	NpyIterContext *npyiter;

} TypeContext;

typedef struct __PyObjectEncoder
{
	JSONObjectEncoder enc;

	// need a separate encoder for labels as USJON always assumes they are strings
	// and using the current encoder would mess up the buffer
	JSONObjectEncoder* labelEncoder;

	// output format style for pandas data types
	int outputFormat;
} PyObjectEncoder;

#define GET_TC(__ptrtc) ((TypeContext *)((__ptrtc)->prv))


enum PRIVATE
{
	PRV_CONV_FUNC,					// Function pointer to converter function
	PRV_CONV_NEWOBJ,				// Any new PyObject created by converter function that should be released by releaseValue
	PRV_ITER_BEGIN_FUNC,		// Function pointer to iterBegin for specific type
	PRV_ITER_END_FUNC,			// Function pointer to iterEnd for specific type
	PRV_ITER_NEXT_FUNC,			// Function pointer to iterNext for specific type
	PRV_ITER_GETVALUE_FUNC,
	PRV_ITER_GETNAME_FUNC,
	PRV_ITER_INDEX,					// Index in the iteration list
	PRV_ITER_SIZE,					// Size of the iteration list
	PRV_ITER_ITEM,					// Current iter item
	PRV_ITER_ITEM_NAME,			// Name of iter item
	PRV_ITER_ITEM_VALUE,		// Value of iteritem
	PRV_ITER_DICTITEMS,
	PRV_ITER_DICTOBJ,
	PRV_ITER_ATTRLIST,
};

struct PyDictIterState
{
	PyObject *keys;
	size_t i;
	size_t sz;
};

enum PANDAS_FORMAT
{
	HEADERS,
	RECORDS,
	INDEXED,
	COLUMN_INDEXED
};

//#define PRINTMARK() fprintf(stderr, "%s: MARK(%d)\n", __FILE__, __LINE__)		
#define PRINTMARK()

void initObjToJSON()
{
	//FIXME: DECREF on these?
	PyDateTime_IMPORT;

	/*
	FIXME: Find the direct function pointer here instead and use it when time conversion is performed */

	meth_timegm = PyString_FromString("timegm");
	mod_calendar = PyImport_ImportModule("calendar");

	Py_INCREF(mod_calendar);

	PyObject *mod_frame = PyImport_ImportModule("pandas.core.frame");
	cls_dataframe = PyObject_GetAttrString(mod_frame, "DataFrame");
	cls_index = PyObject_GetAttrString(mod_frame, "Index");
	cls_series = PyObject_GetAttrString(mod_frame, "Series");
	Py_DECREF(mod_frame);

	/* Initialise numpy API */
	import_array();
}

static void *PyIntToINT32(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
	PyObject *obj = (PyObject *) _obj;
	*((JSINT32 *) outValue) = PyInt_AS_LONG (obj);
	return NULL;
}

static void *PyIntToINT64(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
	PyObject *obj = (PyObject *) _obj;
	*((JSINT64 *) outValue) = PyInt_AS_LONG (obj);
	return NULL;
}

static void *PyLongToINT64(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
	PyObject *obj = (PyObject *) _obj;
	*((JSINT64 *) outValue) = GET_TC(tc)->longValue;
	return NULL;
}

static void *NpyHalfToDOUBLE(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
	PyObject *obj = (PyObject *) _obj;
	unsigned long ctype;
	PyArray_ScalarAsCtype(obj, &ctype);
	*((double *) outValue) = npy_half_to_double (ctype);
	return NULL;
}

static void *NpyFloatToDOUBLE(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
	PyObject *obj = (PyObject *) _obj;
	PyArray_CastScalarToCtype(obj, outValue, PyArray_DescrFromType(NPY_DOUBLE));
	return NULL;
}

static void *PyFloatToDOUBLE(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
	PyObject *obj = (PyObject *) _obj;
	*((double *) outValue) = PyFloat_AS_DOUBLE (obj);
	return NULL;
}

static void *PyStringToUTF8(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
	PyObject *obj = (PyObject *) _obj;
	*_outLen = PyString_GET_SIZE(obj);
	return PyString_AS_STRING(obj);
}

static void *PyUnicodeToUTF8(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
	PyObject *obj = (PyObject *) _obj;
	PyObject *newObj = PyUnicode_EncodeUTF8 (PyUnicode_AS_UNICODE(obj), PyUnicode_GET_SIZE(obj), NULL);

	GET_TC(tc)->newObj = newObj;

	*_outLen = PyString_GET_SIZE(newObj);
	return PyString_AS_STRING(newObj);
}

static void *PyDateTimeToINT64(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
	PyObject *obj = (PyObject *) _obj;

	PyObject* timetuple = PyObject_CallMethod(obj, "utctimetuple", NULL);
	PyObject* unixTimestamp = PyObject_CallMethodObjArgs(mod_calendar, meth_timegm, timetuple, NULL);
	
	*( (JSINT64 *) outValue) = PyLong_AsLongLong (unixTimestamp);
	Py_DECREF(timetuple);
	Py_DECREF(unixTimestamp);
	return NULL;
}

static void *PyDateToINT64(JSOBJ _obj, JSONTypeContext *tc, void *outValue, size_t *_outLen)
{
	PyObject *obj = (PyObject *) _obj;

	PyObject* timetuple = PyTuple_New(6);
	PyObject* year = PyObject_GetAttrString(obj, "year");
	PyObject* month = PyObject_GetAttrString(obj, "month");
	PyObject* day = PyObject_GetAttrString(obj, "day");
	PyObject* unixTimestamp;

	PyTuple_SET_ITEM(timetuple, 0, year);
	PyTuple_SET_ITEM(timetuple, 1, month);
	PyTuple_SET_ITEM(timetuple, 2, day);
	PyTuple_SET_ITEM(timetuple, 3, PyInt_FromLong(0));
	PyTuple_SET_ITEM(timetuple, 4, PyInt_FromLong(0));
	PyTuple_SET_ITEM(timetuple, 5, PyInt_FromLong(0));

	unixTimestamp = PyObject_CallMethodObjArgs(mod_calendar, meth_timegm, timetuple, NULL);

	*( (JSINT64 *) outValue) = PyLong_AsLongLong (unixTimestamp);
	Py_DECREF(timetuple);
	Py_DECREF(unixTimestamp);
	return NULL;
}

//=============================================================================
// Numpy array iteration functions 
//=============================================================================
void NpyArr_iterBegin(JSOBJ _obj, JSONTypeContext *tc)
{
	PyArrayObject *obj;
	if (GET_TC(tc)->newObj)
	{
		obj = (PyArrayObject *) GET_TC(tc)->newObj;
	}
	else
	{
		obj = (PyArrayObject *) _obj;
	}

	if (PyArray_SIZE(obj) > 0)
	{
		NpyIterContext *npyiter = malloc(sizeof(NpyIterContext));
		
		if (!npyiter)
		{
			PyErr_NoMemory();
			return;
		}

		if (!GET_TC(tc)->npyiterOrder)
		{
			GET_TC(tc)->npyiterOrder = NPY_CORDER;
		}

		GET_TC(tc)->npyiter = npyiter;
		npyiter->order = GET_TC(tc)->npyiterOrder;
		npyiter->array = obj;
		npyiter->iter = NpyIter_New(
				(PyArrayObject *) obj, 
				NPY_ITER_READONLY | NPY_ITER_REFS_OK | NPY_ITER_MULTI_INDEX,
				npyiter->order,
				NPY_NO_CASTING,
				NULL); 
		if (!npyiter)
		{
			return;
		}
		npyiter->indexfunc = NpyIter_GetGetMultiIndex(npyiter->iter, NULL);
		npyiter->dataptr = NpyIter_GetDataPtrArray(npyiter->iter);
		npyiter->iternext = NpyIter_GetIterNext(npyiter->iter, NULL);
		npyiter->ndim = PyArray_NDIM(obj);
		npyiter->level = 1;
		npyiter->closelevel = 0;

		npyiter->columnLabels = GET_TC(tc)->columnLabels;
		npyiter->rowLabels = GET_TC(tc)->rowLabels;
		if (npyiter->order == NPY_CORDER) 
		{
			npyiter->rowLabelsDim = 0;
			npyiter->columnLabelsDim = npyiter->ndim-1;
		}
		else
		{
			npyiter->rowLabelsDim = npyiter->ndim-1;
			npyiter->columnLabelsDim = 0;
		}

	}
	PRINTMARK();
}

void NpyArr_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
	if (GET_TC(tc)->npyiter)
	{ 
		NpyIter_Deallocate(GET_TC(tc)->npyiter->iter);
		free(GET_TC(tc)->npyiter);
	}
	Py_XDECREF(GET_TC(tc)->newObj);
	PRINTMARK();
}

void NpyIter_iterBegin(JSOBJ obj, JSONTypeContext *tc)
{
	PRINTMARK();
}

void NpyIter_encodeLabel(JSONTypeContext* tc, NpyIterContext* npyiter, PyObject* labels, npy_intp dim)
{
	PRINTMARK();
	// get the label object from the label array
	npyiter->indexfunc(npyiter->iter, npyiter->index);
	void* labelItem = PyArray_GETPTR1(labels, npyiter->index[dim]);
	PyObject* labelObj = PyArray_GETITEM(labels, labelItem);
	JSONObjectEncoder* labelEncoder = ((PyObjectEncoder*)tc->encoder)->labelEncoder;

	// encode the label, ensuring that we trim off the quotes surrounding the result
	GET_TC(tc)->citemName = JSON_EncodeObject(labelObj, labelEncoder, labelEncoder->start, labelEncoder->end - labelEncoder->start) + 1;
	*(labelEncoder->offset-2) = '\0';
}

int NpyIter_iterNext(JSOBJ _obj, JSONTypeContext *tc)
{
	NpyIterContext *npyiter;
	if (PyCapsule_CheckExact(_obj))
	{
		npyiter = PyCapsule_GetPointer(_obj, "ujsonNpyIterCtxt");
	}
	else
	{
		npyiter = GET_TC(tc)->npyiter;
	}

	if (!npyiter || !npyiter->iternext)
	{
		PRINTMARK();
		return 0;
	}

	if (npyiter->closelevel)
	{
		npyiter->closelevel--;
		npyiter->level--;
		PRINTMARK();
		return 0;
	}

	if (npyiter->level < npyiter->ndim)
	{
		if (npyiter->rowLabels) 
		{
			NpyIter_encodeLabel(tc, npyiter, npyiter->rowLabels, npyiter->rowLabelsDim);
		}
		npyiter->level++;
		GET_TC(tc)->itemValue = PyCapsule_New(npyiter, "ujsonNpyIterCtxt", NULL);
		PRINTMARK();
		return 1;
	}

	GET_TC(tc)->itemValue = PyArray_GETITEM(npyiter->array, *npyiter->dataptr);

	if (npyiter->columnLabels) 
	{
		NpyIter_encodeLabel(tc, npyiter, npyiter->columnLabels, npyiter->columnLabelsDim);
	}

	if (!npyiter->iternext(npyiter->iter))
	{
		npyiter->iternext = NULL;
	}
	else
	{
		npyiter->indexfunc(npyiter->iter, npyiter->index);
		int i;
		if (npyiter->order == NPY_CORDER)
		{
			for (i = npyiter->ndim-1; npyiter->index[i] == 0; i--)
			{
				npyiter->closelevel++;
			}
		}
		else
		{
			for (i = 0; npyiter->index[i] == 0; i++)
			{
				npyiter->closelevel++;
			}
		}
	}

	PRINTMARK();
	return 1;
}

JSOBJ NpyIter_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
	PRINTMARK();
	return GET_TC(tc)->itemValue;
}

char *NpyIter_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
	PRINTMARK();
	*outLen = strlen(GET_TC(tc)->citemName);
	return GET_TC(tc)->citemName;
}

void NpyIter_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
	Py_CLEAR(obj);
	PRINTMARK();
}

//=============================================================================
// Tuple iteration functions 
// itemValue is borrowed reference, no ref counting
//=============================================================================
void Tuple_iterBegin(JSOBJ obj, JSONTypeContext *tc)
{
	GET_TC(tc)->index = 0;
	GET_TC(tc)->size = PyTuple_GET_SIZE( (PyObject *) obj);
	GET_TC(tc)->itemValue = NULL;
}

int Tuple_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
	PyObject *item;

	if (GET_TC(tc)->index >= GET_TC(tc)->size)
	{
		return 0;
	}

	item = PyTuple_GET_ITEM (obj, GET_TC(tc)->index);

	GET_TC(tc)->itemValue = item;
	GET_TC(tc)->index ++;
	return 1;
}

void Tuple_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
}

JSOBJ Tuple_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
	return GET_TC(tc)->itemValue;
}

char *Tuple_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
	return NULL;
}

//=============================================================================
// Dir iteration functions 
// itemName ref is borrowed from PyObject_Dir (attrList). No refcount
// itemValue ref is from PyObject_GetAttr. Ref counted
//=============================================================================
void Dir_iterBegin(JSOBJ obj, JSONTypeContext *tc)
{
	GET_TC(tc)->attrList = PyObject_Dir(obj); 
	GET_TC(tc)->index = 0;
	GET_TC(tc)->size = PyList_GET_SIZE(GET_TC(tc)->attrList);
	PRINTMARK();
}

void Dir_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
	if (GET_TC(tc)->itemValue)
	{
		Py_DECREF(GET_TC(tc)->itemValue);
		GET_TC(tc)->itemValue = NULL;
	}

	Py_DECREF( (PyObject *) GET_TC(tc)->attrList);
	PRINTMARK();
}

int Dir_iterNext(JSOBJ _obj, JSONTypeContext *tc)
{
	PyObject *obj = (PyObject *) _obj;
	PyObject *itemValue = GET_TC(tc)->itemValue;
	PyObject *itemName = NULL;


	if (itemValue)
	{
		Py_DECREF(GET_TC(tc)->itemValue);
		GET_TC(tc)->itemValue = itemValue = NULL;
	}

	for (; GET_TC(tc)->index  < GET_TC(tc)->size; GET_TC(tc)->index ++)
	{
		PyObject* attr = PyList_GET_ITEM(GET_TC(tc)->attrList, GET_TC(tc)->index);
		char* attrStr = PyString_AS_STRING(attr);

		if (attrStr[0] == '_')
		{
			PRINTMARK();
			continue;
		}

		itemValue = PyObject_GetAttr(obj, attr);
		if (itemValue == NULL)
		{
			PyErr_Clear();
			PRINTMARK();
			continue;
		}

		if (PyCallable_Check(itemValue))
		{
			Py_DECREF(itemValue);
			PRINTMARK();
			continue;
		}

		PRINTMARK();
		itemName = attr;
		break;
	}

	if (itemName == NULL)
	{
		GET_TC(tc)->index = GET_TC(tc)->size;
		GET_TC(tc)->itemValue = NULL;
		return 0;
	}

	GET_TC(tc)->itemName = itemName;
	GET_TC(tc)->itemValue = itemValue;
	GET_TC(tc)->index ++;
	
	PRINTMARK();
	return 1;
}



JSOBJ Dir_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
	PRINTMARK();
	return GET_TC(tc)->itemValue;
}

char *Dir_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
	PRINTMARK();
	*outLen = PyString_GET_SIZE(GET_TC(tc)->itemName);
	return PyString_AS_STRING(GET_TC(tc)->itemName);
}




//=============================================================================
// List iteration functions 
// itemValue is borrowed from object (which is list). No refcounting
//=============================================================================
void List_iterBegin(JSOBJ obj, JSONTypeContext *tc)
{
	GET_TC(tc)->index =  0;
	GET_TC(tc)->size = PyList_GET_SIZE( (PyObject *) obj);
}

int List_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
	if (GET_TC(tc)->index >= GET_TC(tc)->size)
	{
		PRINTMARK();
		return 0;
	}

	GET_TC(tc)->itemValue = PyList_GET_ITEM (obj, GET_TC(tc)->index);
	GET_TC(tc)->index ++;
	return 1;
}

void List_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
}

JSOBJ List_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
	return GET_TC(tc)->itemValue;
}

char *List_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
	return NULL;
}

//=============================================================================
// pandas Index iteration functions 
//=============================================================================
void Index_iterBegin(JSOBJ obj, JSONTypeContext *tc)
{
	GET_TC(tc)->index = 0;
	GET_TC(tc)->citemName = malloc(20 * sizeof(char));
	if (!GET_TC(tc)->citemName)
	{
		PyErr_NoMemory();
	}
	PRINTMARK();
}

int Index_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
	if (!GET_TC(tc)->citemName)
	{
		return 0;
	}

	Py_ssize_t index = GET_TC(tc)->index;
	Py_XDECREF(GET_TC(tc)->itemValue);
	if (index == 0)
	{
		memcpy(GET_TC(tc)->citemName, "name", 5);
		GET_TC(tc)->itemValue = PyObject_GetAttrString(obj, "name");
	}
	else
	if (index == 1)
	{
		memcpy(GET_TC(tc)->citemName, "data", 5);
		GET_TC(tc)->itemValue = PyObject_GetAttrString(obj, "values");
	}
	else 
	{
		PRINTMARK();
		return 0;
	}

	GET_TC(tc)->index++;
	PRINTMARK();
	return 1;
}

void Index_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
	if (GET_TC(tc)->citemName)
	{
		free(GET_TC(tc)->citemName);
	}
	PRINTMARK();
}

JSOBJ Index_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
	return GET_TC(tc)->itemValue;
}

char *Index_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
	*outLen = strlen(GET_TC(tc)->citemName);
	return GET_TC(tc)->citemName;
}       

//=============================================================================
// pandas Series iteration functions 
//=============================================================================
void Series_iterBegin(JSOBJ obj, JSONTypeContext *tc)
{
	GET_TC(tc)->index = 0;
	GET_TC(tc)->citemName = malloc(20 * sizeof(char));
	if (!GET_TC(tc)->citemName)
	{
		PyErr_NoMemory();
	}
	PRINTMARK();
}

int Series_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
	if (!GET_TC(tc)->citemName)
	{
		return 0;
	}

	Py_ssize_t index = GET_TC(tc)->index;
	Py_XDECREF(GET_TC(tc)->itemValue);
	if (index == 0)
	{
		memcpy(GET_TC(tc)->citemName, "name", 5);
		GET_TC(tc)->itemValue = PyObject_GetAttrString(obj, "name");
	}
	else
	if (index == 1)
	{
		memcpy(GET_TC(tc)->citemName, "index", 6);
		GET_TC(tc)->itemValue = PyObject_GetAttrString(obj, "index");
	}
	else
	if (index == 2)
	{
		memcpy(GET_TC(tc)->citemName, "data", 5);
		GET_TC(tc)->itemValue = PyObject_GetAttrString(obj, "values");
	}
	else 
	{
		PRINTMARK();
		return 0;
	}

	GET_TC(tc)->index++;
	PRINTMARK();
	return 1;
}

void Series_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
	if (GET_TC(tc)->citemName)
	{
		free(GET_TC(tc)->citemName);
	}
	PRINTMARK();
}

JSOBJ Series_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
	return GET_TC(tc)->itemValue;
}

char *Series_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
	*outLen = strlen(GET_TC(tc)->citemName);
	return GET_TC(tc)->citemName;
}       

//=============================================================================
// pandas DataFrame iteration functions 
//=============================================================================
void DataFrame_iterBegin(JSOBJ obj, JSONTypeContext *tc)
{
	GET_TC(tc)->index = 0;
	GET_TC(tc)->citemName = malloc(20 * sizeof(char));
	if (!GET_TC(tc)->citemName)
	{
		PyErr_NoMemory();
	}
	PRINTMARK();
}

int DataFrame_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
	if (!GET_TC(tc)->citemName)
	{
		return 0;
	}

	Py_ssize_t index = GET_TC(tc)->index;
	Py_XDECREF(GET_TC(tc)->itemValue);
	if (index == 0)
	{
		memcpy(GET_TC(tc)->citemName, "columns", 8);
		GET_TC(tc)->itemValue = PyObject_GetAttrString(obj, "columns");
	}
	else
	if (index == 1)
	{
		memcpy(GET_TC(tc)->citemName, "index", 6);
		GET_TC(tc)->itemValue = PyObject_GetAttrString(obj, "index");
	}
	else
	if (index == 2)
	{
		memcpy(GET_TC(tc)->citemName, "data", 5);
		GET_TC(tc)->itemValue = PyObject_GetAttrString(obj, "values");
	}
	else 
	{
		PRINTMARK();
		return 0;
	}

	GET_TC(tc)->index++;
	PRINTMARK();
	return 1;
}

void DataFrame_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
	if (GET_TC(tc)->citemName)
	{
		free(GET_TC(tc)->citemName);
	}
	PRINTMARK();
}

JSOBJ DataFrame_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
	return GET_TC(tc)->itemValue;
}

char *DataFrame_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
	*outLen = strlen(GET_TC(tc)->citemName);
	return GET_TC(tc)->citemName;
}       

//=============================================================================
// Dict iteration functions 
// itemName might converted to string (Python_Str). Do refCounting
// itemValue is borrowed from object (which is dict). No refCounting
//=============================================================================
void Dict_iterBegin(JSOBJ obj, JSONTypeContext *tc)
{
	GET_TC(tc)->index = 0;
	PRINTMARK();
}

int Dict_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
	if (GET_TC(tc)->itemName)
	{
		Py_DECREF(GET_TC(tc)->itemName);
		GET_TC(tc)->itemName = NULL;
	}


	if (!PyDict_Next ( (PyObject *)GET_TC(tc)->dictObj, &GET_TC(tc)->index, &GET_TC(tc)->itemName, &GET_TC(tc)->itemValue))
	{
		PRINTMARK();
		return 0;
	}

	if (PyUnicode_Check(GET_TC(tc)->itemName))
	{
		GET_TC(tc)->itemName = PyUnicode_EncodeUTF8 (
			PyUnicode_AS_UNICODE(GET_TC(tc)->itemName),
			PyUnicode_GET_SIZE(GET_TC(tc)->itemName),
			NULL
		);
	}
	else
	if (!PyString_Check(GET_TC(tc)->itemName))
	{
		GET_TC(tc)->itemName = PyObject_Str(GET_TC(tc)->itemName);
	}
	else 
	{
		Py_INCREF(GET_TC(tc)->itemName);
	}
	PRINTMARK();
	return 1;
}

void Dict_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
	if (GET_TC(tc)->itemName)
	{
		Py_DECREF(GET_TC(tc)->itemName);
		GET_TC(tc)->itemName = NULL;
	}
	Py_DECREF(GET_TC(tc)->dictObj);
	PRINTMARK();
}

JSOBJ Dict_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
	return GET_TC(tc)->itemValue;
}

char *Dict_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
	*outLen = PyString_GET_SIZE(GET_TC(tc)->itemName);
	return PyString_AS_STRING(GET_TC(tc)->itemName);
}

void Object_beginTypeContext (PyObject *obj, JSONTypeContext *tc)
{
	TypeContext *pc = (TypeContext *) tc->prv;
	PyObjectEncoder* enc = (PyObjectEncoder*) tc->encoder;
	PyObject *toDictFunc;

	int i;
	for (i = 0; i < 32; i++) 
	{
		tc->prv[i] = 0;
	}

	if (PyIter_Check(obj) || PyArray_Check(obj))
	{
		goto ISITERABLE;
	}
	else 
	if (PyCapsule_IsValid(obj, "ujsonNpyIterCtxt"))
	{
		NpyIterContext *npyiter = PyCapsule_GetPointer(obj, "ujsonNpyIterCtxt");
		tc->type = (npyiter->columnLabels ? JT_OBJECT : JT_ARRAY);
		pc->iterBegin = NpyIter_iterBegin;
		pc->iterEnd = NpyIter_iterEnd;
		pc->iterNext = NpyIter_iterNext;
		pc->iterGetValue = NpyIter_iterGetValue;
		pc->iterGetName = NpyIter_iterGetName;
		return;
	}

	if (PyBool_Check(obj))
	{
		PRINTMARK();
		tc->type = (obj == Py_True) ? JT_TRUE : JT_FALSE;
		return;
	}
	else
	if (PyInt_Check(obj))
	{
		PRINTMARK();
#ifdef _LP64
		pc->PyTypeToJSON = PyIntToINT64; tc->type = JT_LONG;
#else
		pc->PyTypeToJSON = PyIntToINT32; tc->type = JT_INT;
#endif
		return;
	}
	else 
	if (PyLong_Check(obj))
	{
		PyObject *exc;

		PRINTMARK();
		pc->PyTypeToJSON = PyLongToINT64; 
		tc->type = JT_LONG;
		GET_TC(tc)->longValue = PyLong_AsLongLong(obj);

		exc = PyErr_Occurred();

		if (exc && PyErr_ExceptionMatches(PyExc_OverflowError))
		{
			PRINTMARK();
			tc->type = JT_INVALID;
			return;
		}

		return;
	}
	else 
	if (PyArray_IsScalar(obj, Integer))
	{
		PyObject *exc;

		PRINTMARK();
		pc->PyTypeToJSON = PyLongToINT64; 
		tc->type = JT_LONG;
		PyArray_CastScalarToCtype(obj, &(GET_TC(tc)->longValue), PyArray_DescrFromType(NPY_LONG));

		exc = PyErr_Occurred();

		if (exc && PyErr_ExceptionMatches(PyExc_OverflowError))
		{
			PRINTMARK();
			tc->type = JT_INVALID;
			return;
		}

		return;
	}
	else
	if (PyString_Check(obj))
	{
		PRINTMARK();
		pc->PyTypeToJSON = PyStringToUTF8; tc->type = JT_UTF8;
		return;
	}
	else
	if (PyUnicode_Check(obj))
	{
		PRINTMARK();
		pc->PyTypeToJSON = PyUnicodeToUTF8; tc->type = JT_UTF8;
		return;
	}
	else
	if (PyFloat_Check(obj))
	{
		PRINTMARK();
		pc->PyTypeToJSON = PyFloatToDOUBLE; tc->type = JT_DOUBLE;
		return;
	}
	else
	if (PyArray_IsScalar(obj, Float))
	{
		PRINTMARK();
		pc->PyTypeToJSON = NpyFloatToDOUBLE; tc->type = JT_DOUBLE;
		return;
	}
	else
	if (PyArray_IsScalar(obj, Half))
	{
		PRINTMARK();
		pc->PyTypeToJSON = NpyHalfToDOUBLE; tc->type = JT_DOUBLE;
		return;
	}
	else 
	if (PyDateTime_Check(obj))
	{
		PRINTMARK();
		pc->PyTypeToJSON = PyDateTimeToINT64; tc->type = JT_LONG;
		return;
	}
	else 
	if (PyDate_Check(obj))
	{
		PRINTMARK();
		pc->PyTypeToJSON = PyDateToINT64; tc->type = JT_LONG;
		return;
	}
	else
	if (obj == Py_None)
	{
		PRINTMARK();
		tc->type = JT_NULL;
		return;
	}


ISITERABLE:

	if (PyDict_Check(obj))
	{
		PRINTMARK();
		tc->type = JT_OBJECT;
		pc->iterBegin = Dict_iterBegin;
		pc->iterEnd = Dict_iterEnd;
		pc->iterNext = Dict_iterNext;
		pc->iterGetValue = Dict_iterGetValue;
		pc->iterGetName = Dict_iterGetName;
		pc->dictObj = obj;
		Py_INCREF(obj);

		return;
	}
	else
	if (PyList_Check(obj))
	{
		PRINTMARK();
		tc->type = JT_ARRAY;
		pc->iterBegin = List_iterBegin;
		pc->iterEnd = List_iterEnd;
		pc->iterNext = List_iterNext;
		pc->iterGetValue = List_iterGetValue;
		pc->iterGetName = List_iterGetName;
		return;
	}
	else
	if (PyTuple_Check(obj))
	{
		PRINTMARK();
		tc->type = JT_ARRAY;
		pc->iterBegin = Tuple_iterBegin;
		pc->iterEnd = Tuple_iterEnd;
		pc->iterNext = Tuple_iterNext;
		pc->iterGetValue = Tuple_iterGetValue;
		pc->iterGetName = Tuple_iterGetName;
		return;
	}
	else
	if (PyObject_TypeCheck(obj, cls_index))
	{
		if (enc->outputFormat == HEADERS) 
		{
			PRINTMARK();
			tc->type = JT_OBJECT;
			pc->iterBegin = Index_iterBegin;
			pc->iterEnd = Index_iterEnd;
			pc->iterNext = Index_iterNext;
			pc->iterGetValue = Index_iterGetValue;
			pc->iterGetName = Index_iterGetName;
			return;
		}

		tc->type = JT_ARRAY;
		pc->newObj = PyObject_GetAttrString(obj, "values");
		pc->iterBegin = NpyArr_iterBegin;
		pc->iterEnd = NpyArr_iterEnd;
		pc->iterNext = NpyIter_iterNext;
		pc->iterGetValue = NpyIter_iterGetValue;
		pc->iterGetName = NpyIter_iterGetName;
		return;
	}
	else
	if (PyObject_TypeCheck(obj, cls_series))
	{
		if (enc->outputFormat == HEADERS) 
		{
			PRINTMARK();
			enc->outputFormat = RECORDS; // keep contained index data type simple
			tc->type = JT_OBJECT;
			pc->iterBegin = Series_iterBegin;
			pc->iterEnd = Series_iterEnd;
			pc->iterNext = Series_iterNext;
			pc->iterGetValue = Series_iterGetValue;
			pc->iterGetName = Series_iterGetName;
			return;
		}

		if (enc->outputFormat == INDEXED || enc->outputFormat == COLUMN_INDEXED)
		{
			PRINTMARK();
			tc->type = JT_OBJECT;
			pc->columnLabels = PyObject_GetAttrString(obj, "index");
		}
		else
		{
			PRINTMARK();
			tc->type = JT_ARRAY;
		}
		pc->newObj = PyObject_GetAttrString(obj, "values");
		pc->iterBegin = NpyArr_iterBegin;
		pc->iterEnd = NpyArr_iterEnd;
		pc->iterNext = NpyIter_iterNext;
		pc->iterGetValue = NpyIter_iterGetValue;
		pc->iterGetName = NpyIter_iterGetName;
		return;
	}
	else
	if (PyArray_Check(obj))
	{
		PRINTMARK();
		tc->type = JT_ARRAY;
		pc->iterBegin = NpyArr_iterBegin;
		pc->iterEnd = NpyArr_iterEnd;
		pc->iterNext = NpyIter_iterNext;
		pc->iterGetValue = NpyIter_iterGetValue;
		pc->iterGetName = NpyIter_iterGetName;
		return;
	}
	else
	if (PyObject_TypeCheck(obj, cls_dataframe))
	{
		if (enc->outputFormat == HEADERS) 
		{
			PRINTMARK();
			enc->outputFormat = RECORDS; // keep contained index data type simple
			tc->type = JT_OBJECT;
			pc->iterBegin = DataFrame_iterBegin;
			pc->iterEnd = DataFrame_iterEnd;
			pc->iterNext = DataFrame_iterNext;
			pc->iterGetValue = DataFrame_iterGetValue;
			pc->iterGetName = DataFrame_iterGetName;
			return;
		}

		if (enc->outputFormat == RECORDS)
		{
			PRINTMARK();
			tc->type = JT_ARRAY;
			pc->columnLabels = PyObject_GetAttrString(obj, "columns");
		}
		else 
		if (enc->outputFormat == INDEXED)
		{
			PRINTMARK();
			tc->type = JT_OBJECT;
			pc->rowLabels = PyObject_GetAttrString(obj, "index");
			pc->columnLabels = PyObject_GetAttrString(obj, "columns");
		}
		else 
		if (enc->outputFormat == COLUMN_INDEXED)
		{
			PRINTMARK();
			tc->type = JT_OBJECT;
			pc->rowLabels = PyObject_GetAttrString(obj, "columns");
			pc->columnLabels = PyObject_GetAttrString(obj, "index");
			pc->npyiterOrder = NPY_FORTRANORDER;
		}
		pc->newObj = PyObject_GetAttrString(obj, "values");
		pc->iterBegin = NpyArr_iterBegin;
		pc->iterEnd = NpyArr_iterEnd;
		pc->iterNext = NpyIter_iterNext;
		pc->iterGetValue = NpyIter_iterGetValue;
		pc->iterGetName = NpyIter_iterGetName;
		return;
	}


	toDictFunc = PyObject_GetAttrString(obj, "toDict");

	if (toDictFunc)
	{
		PyObject* tuple = PyTuple_New(0);
		PyObject* toDictResult = PyObject_Call(toDictFunc, tuple, NULL);
		Py_DECREF(tuple);
		Py_DECREF(toDictFunc);

		if (toDictResult == NULL)
		{
			PyErr_Clear();
			tc->type = JT_NULL;
			return;
		}

		if (!PyDict_Check(toDictResult))
		{
			Py_DECREF(toDictResult);
			tc->type = JT_NULL;
			return;
		}

		PRINTMARK();
		tc->type = JT_OBJECT;
		pc->iterBegin = Dict_iterBegin;
		pc->iterEnd = Dict_iterEnd;
		pc->iterNext = Dict_iterNext;
		pc->iterGetValue = Dict_iterGetValue;
		pc->iterGetName = Dict_iterGetName;
		pc->dictObj = toDictResult;
		return;
	}

	PyErr_Clear();

	tc->type = JT_OBJECT;
	pc->iterBegin = Dir_iterBegin;
	pc->iterEnd = Dir_iterEnd;
	pc->iterNext = Dir_iterNext;
	pc->iterGetValue = Dir_iterGetValue;
	pc->iterGetName = Dir_iterGetName;

	return;
}


void Object_endTypeContext(JSOBJ obj, JSONTypeContext *tc)
{
	Py_XDECREF(GET_TC(tc)->newObj);
}

const char *Object_getStringValue(JSOBJ obj, JSONTypeContext *tc, size_t *_outLen)
{
	return GET_TC(tc)->PyTypeToJSON (obj, tc, NULL, _outLen);
}

JSINT64 Object_getLongValue(JSOBJ obj, JSONTypeContext *tc)
{
	JSINT64 ret;
	GET_TC(tc)->PyTypeToJSON (obj, tc, &ret, NULL);

	return ret;
}

JSINT32 Object_getIntValue(JSOBJ obj, JSONTypeContext *tc)
{
	JSINT32 ret;
	GET_TC(tc)->PyTypeToJSON (obj, tc, &ret, NULL);
	return ret;
}


double Object_getDoubleValue(JSOBJ obj, JSONTypeContext *tc)
{
	double ret;
	GET_TC(tc)->PyTypeToJSON (obj, tc, &ret, NULL);
	return ret;
}

static void Object_releaseObject(JSOBJ *_obj)
{
	Py_DECREF( (PyObject *) _obj);
}



void Object_iterBegin(JSOBJ obj, JSONTypeContext *tc)
{
	GET_TC(tc)->iterBegin(obj, tc);
}

int Object_iterNext(JSOBJ obj, JSONTypeContext *tc)
{
	return GET_TC(tc)->iterNext(obj, tc);
}

void Object_iterEnd(JSOBJ obj, JSONTypeContext *tc)
{
	GET_TC(tc)->iterEnd(obj, tc);
}

JSOBJ Object_iterGetValue(JSOBJ obj, JSONTypeContext *tc)
{
	return GET_TC(tc)->iterGetValue(obj, tc);
}

char *Object_iterGetName(JSOBJ obj, JSONTypeContext *tc, size_t *outLen)
{
	return GET_TC(tc)->iterGetName(obj, tc, outLen);
}


PyObject* objToJSON(PyObject* self, PyObject *args, PyObject *kwargs)
{
	static char *kwlist[] = { "obj", "ensure_ascii", "double_precision", "format", NULL};

	char buffer[65536];
	char labelBuffer[1024];
	char *ret;
	PyObject *newobj;
	PyObject *oinput = NULL;
	PyObject *oensureAscii = NULL;
	char *sFormat = NULL;
	int idoublePrecision = 5; // default double precision setting

	PyObjectEncoder pyEncoder = 
	{
		Object_beginTypeContext,	//void (*beginTypeContext)(JSOBJ obj, JSONTypeContext *tc);
		Object_endTypeContext, //void (*endTypeContext)(JSOBJ obj, JSONTypeContext *tc);
		Object_getStringValue, //const char *(*getStringValue)(JSOBJ obj, JSONTypeContext *tc, size_t *_outLen);
		Object_getLongValue, //JSLONG (*getLongValue)(JSOBJ obj, JSONTypeContext *tc);
		Object_getIntValue, //JSLONG (*getLongValue)(JSOBJ obj, JSONTypeContext *tc);
		Object_getDoubleValue, //double (*getDoubleValue)(JSOBJ obj, JSONTypeContext *tc);
		Object_iterBegin, //JSPFN_ITERBEGIN iterBegin;
		Object_iterNext, //JSPFN_ITERNEXT iterNext;
		Object_iterEnd, //JSPFN_ITEREND iterEnd;
		Object_iterGetValue, //JSPFN_ITERGETVALUE iterGetValue;
		Object_iterGetName, //JSPFN_ITERGETNAME iterGetName;
		Object_releaseObject, //void (*releaseValue)(JSONTypeContext *ti);
		PyObject_Malloc, //JSPFN_MALLOC malloc;
		PyObject_Realloc, //JSPFN_REALLOC realloc;
		PyObject_Free, //JSPFN_FREE free;
		-1, //recursionMax
		idoublePrecision,
		1, //forceAscii
	};
	JSONObjectEncoder* encoder = (JSONObjectEncoder*) &pyEncoder;

	PyObjectEncoder labelEncoder = pyEncoder;
	JSONObjectEncoder* jsonLabelEncoder = (JSONObjectEncoder*) &labelEncoder;
	jsonLabelEncoder->start = &labelBuffer;
	jsonLabelEncoder->end = jsonLabelEncoder->start + sizeof(labelBuffer);

	pyEncoder.labelEncoder = &labelEncoder;
	pyEncoder.outputFormat = COLUMN_INDEXED;

	PRINTMARK();

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|Ois", kwlist, &oinput, &oensureAscii, &idoublePrecision, &sFormat))
	{
		return NULL;
	}

	if (sFormat != NULL)
	{
		if (strcmp(sFormat, "records") == 0)
		{
			pyEncoder.outputFormat = RECORDS;
		} 
		else
		if (strcmp(sFormat, "indexed") == 0)
		{
			pyEncoder.outputFormat = INDEXED;
		}
		else
		if (strcmp(sFormat, "headers") == 0)
		{
			pyEncoder.outputFormat = HEADERS;
		}
		else
		if (strcmp(sFormat, "column_indexed") != 0)
		{
			PyErr_Format (PyExc_ValueError, "Invalid value '%s' for option 'format'", sFormat);
			return NULL;
		}
	}

	if (oensureAscii != NULL && !PyObject_IsTrue(oensureAscii))
	{
		encoder->forceASCII = 0;
	}

	encoder->doublePrecision = idoublePrecision;

	PRINTMARK();
	ret = JSON_EncodeObject (oinput, encoder, buffer, sizeof (buffer));
	PRINTMARK();

	if (PyErr_Occurred())
	{
		return NULL;
	}

	if (encoder->errorMsg)
	{
		if (ret != buffer)
		{
			encoder->free (ret);
		}

		PyErr_Format (PyExc_OverflowError, "%s", encoder->errorMsg);
		return NULL;
	}

	newobj = PyString_FromString (ret);

	if (ret != buffer)
	{
		encoder->free (ret);
	}

	PRINTMARK();

	return newobj;
}

PyObject* objToJSONFile(PyObject* self, PyObject *args, PyObject *kwargs)
{
	PyObject *data;
	PyObject *file;
	PyObject *string;
	PyObject *write;
	PyObject *argtuple;

	PRINTMARK();

	if (!PyArg_ParseTuple (args, "OO", &data, &file)) {
		return NULL;
	}

	if (!PyObject_HasAttrString (file, "write"))
	{
		PyErr_Format (PyExc_TypeError, "expected file");
		return NULL;
	}

	write = PyObject_GetAttrString (file, "write");

	if (!PyCallable_Check (write)) {
		Py_XDECREF(write);
		PyErr_Format (PyExc_TypeError, "expected file");
		return NULL;
	}

	argtuple = PyTuple_Pack(1, data);

	string = objToJSON (self, argtuple, kwargs);

	if (string == NULL)
	{
		Py_XDECREF(write);
		Py_XDECREF(argtuple);
		return NULL;
	}

	Py_XDECREF(argtuple);

	argtuple = PyTuple_Pack (1, string);
	if (argtuple == NULL)
	{
		Py_XDECREF(write);
		return NULL;
	}
	if (PyObject_CallObject (write, argtuple) == NULL)
	{
		Py_XDECREF(write);
		Py_XDECREF(argtuple);
		return NULL;
	}

	Py_XDECREF(write);
	Py_DECREF(argtuple);
	Py_XDECREF(string);

	PRINTMARK();

	Py_RETURN_NONE;
	

}

