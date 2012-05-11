# coding=UTF-8
from os.path import join, dirname
import simplejson
import ujson
import sys
try:
    import json
except ImportError:
    json = simplejson
import cjson
import yajl
from time import time as gettime
import time
import sys
import random

import numpy as np
from pandas import DataFrame, Series

decodeData = ""

"""=========================================================================="""

def ujsonEnc(orient="columns"):
    x = ujson.encode(testObject, orient=orient, ensure_ascii=False)
    #print "ujsonEnc", x

def ujsonCustomEnc(orient="columns"):
    x = ujson.encode(ujTestObject, orient=orient,ensure_ascii=False)
    #print "ujsonEnc", x

def simplejsonEnc():
    x = simplejson.dumps(testObject)
    #print "simplejsonEnc", x

def jsonEnc():
    x = json.dumps(testObject)
    #print "jsonEnc", x

def cjsonEnc():
    x = cjson.encode(testObject)
    #print "cjsonEnc", x

def yajlEnc():
    x = yajl.dumps(testObject)
    #print "cjsonEnc", x

def ujsonEncNP():
    x = ujson.encode(testObjectNP, ensure_ascii=False)
    #print "ujsonEnc", x

def ujsonEncNPConv():
    x = ujson.encode(testObjectNP.tolist(), ensure_ascii=False)
    #print "ujsonEnc", x

def simplejsonEncNP():
    x = simplejson.dumps(testObjectNP.tolist())
    #print "simplejsonEnc", x

def jsonEncNP():
    x = json.dumps(testObjectNP.tolist())
    #print "jsonEnc", x

def cjsonEncNP():
    x = cjson.encode(testObjectNP.tolist())
    #print "cjsonEnc", x

def yajlEncNP():
    x = yajl.dumps(testObjectNP.tolist())
    #print "cjsonEnc", x

def ujsonEncDF(orient="columns"):
    x = ujson.encode(testObjectDF, orient=orient,ensure_ascii=False)
    #print "ujsonEnc", x

def ujsonEncDFConv(orient="columns"):
    x = ujson.encode(testObjectDF.to_dict(), orient=orient,ensure_ascii=False)
    #print "ujsonEnc", x

def simplejsonEncDF():
    x = simplejson.dumps(testObjectDF.to_dict())
    #print "simplejsonEnc", x

def jsonEncDF():
    x = json.dumps(testObjectDF.to_dict())
    #print "jsonEnc", x

def cjsonEncDF():
    x = cjson.encode(testObjectDF.to_dict())
    #print "cjsonEnc", x

def yajlEncDF():
    x = yajl.dumps(testObjectDF.to_dict())
    #print "cjsonEnc", x

"""=========================================================================="""

def ujsonDec():
    x = ujson.decode(decodeData)
    #print "ujsonDec: ", x

def ujsonDecDFVal():
    x = ujson.decode(decodeDataVals)
    DataFrame(x)
    #print "ujsonDec: ", x

def ujsonDecDFValNP():
    x = ujson.decode(decodeDataVals, numpy=True)
    DataFrame(x)
    #print "ujsonDec: ", x

def ujsonDecDFSplit():
    x = ujson.decode(decodeDataSplit)
    DataFrame(**x)
    #print "ujsonDec: ", x

def ujsonDecDFSplitNP():
    x = ujson.decode(decodeDataSplit, numpy=True)
    DataFrame(**x)
    #print "ujsonDec: ", x

def ujsonDecDFRec():
    x = ujson.decode(decodeDataRec)
    DataFrame(x)
    #print "ujsonDec: ", x

def ujsonDecDFRecNP():
    x = ujson.decode(decodeDataRec, numpy=True, labelled=True)
    DataFrame(*x)
    #print "ujsonDec: ", x

def ujsonDecDFIndex():
    x = ujson.decode(decodeDataIdx)
    DataFrame(x).T
    #print "ujsonDec: ", x

def ujsonDecDFIndexNP():
    x = ujson.decode(decodeDataIdx, numpy=True, labelled=True)
    DataFrame(*x)
    #print "ujsonDec: ", x

def ujsonDecDF():
    x = ujson.decode(decodeData)
    DataFrame(x)
    #print "ujsonDec: ", x

def ujsonDecDFNP():
    x = ujson.decode(decodeData, numpy=True, labelled=True)
    DataFrame(x[0].T, x[2], x[1])
    #print "ujsonDec: ", x

def ujsonDecNPConv():
    x = ujson.decode(decodeData)
    np.array(x)
    #print "ujsonDec: ", x

def ujsonDecNP():
    x = ujson.decode(decodeData, numpy=True)
    #print "ujsonDec: ", x

def simplejsonDec():
    x = simplejson.loads(decodeData)
    #print "simplejsonDec: ", x

def jsonDec():
    x = json.loads(decodeData)
    #print "jsonDec: ", x

def cjsonDec():
    x = cjson.decode(decodeData)
    #print "cjsonDec: ", x

def yajlDec():
    x = yajl.loads(decodeData)
    #print "cjsonDec: ", x

"""=========================================================================="""

def timeit_compat_fix(timeit):
    if sys.version_info[:2] >=  (2,6):
        return
    default_number = 1000000
    default_repeat = 3
    if sys.platform == "win32":
        # On Windows, the best timer is time.clock()
        default_timer = time.clock
    else:
        # On most other platforms the best timer is time.time()
        default_timer = time.time
    def repeat(stmt="pass", setup="pass", timer=default_timer,
       repeat=default_repeat, number=default_number):
        """Convenience function to create Timer object and call repeat method."""
        return timeit.Timer(stmt, setup, timer).repeat(repeat, number)
    timeit.repeat = repeat


if __name__ == "__main__":
    import timeit
    timeit_compat_fix(timeit)


print "Ready? Configure affinity and priority, starting in 20..."
time.sleep(20)

print "Array with 256 utf-8 strings :"
testObject = []

for x in xrange(256):
   testObject.append("نظام الحكم سلطاني وراثي في الذكور من ذرية السيد تركي بن سعيد بن سلطان ويشترط فيمن يختار لولاية الحكم من بينهم ان يكون مسلما رشيدا عاقلا ًوابنا شرعيا لابوين عمانيين ")
testObjectNP = np.array(testObject)


COUNT = 2000


print "ujson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEnc()", "from __main__ import ujsonEnc", gettime,10, COUNT)), )
print "ujson encode NP     : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncNP()", "from __main__ import ujsonEncNP", gettime,10, COUNT)), )
print "ujson encode NP CONV: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncNPConv()", "from __main__ import ujsonEncNPConv", gettime,10, COUNT)), )
print "simplejson encode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEnc()", "from __main__ import simplejsonEnc", gettime,10, COUNT)), )
print "simplejson enc CONV : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEncNP()", "from __main__ import simplejsonEncNP", gettime,10, COUNT)), )
print "cjson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEnc()", "from __main__ import cjsonEnc", gettime, 10, COUNT)), )
print "cjson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEncNP()", "from __main__ import cjsonEncNP", gettime, 10, COUNT)), )
print "yajl  encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEnc()", "from __main__ import yajlEnc", gettime, 10, COUNT)), )
print "yajl  encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEncNP()", "from __main__ import yajlEncNP", gettime, 10, COUNT)), )


print 
decodeData = ujson.dumps(testObject)

print "ujson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDec()", "from __main__ import ujsonDec", gettime,10, COUNT)), )
print "ujson decode NP     : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecNP()", "from __main__ import ujsonDecNP", gettime,10, COUNT)), )
print "ujson decode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecNPConv()", "from __main__ import ujsonDecNPConv", gettime,10, COUNT)), )
print "simplejson decode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonDec()", "from __main__ import simplejsonDec", gettime,10, COUNT)), )
print "cjson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonDec()", "from __main__ import cjsonDec", gettime,10, COUNT)), )
print "yajl decode         : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlDec()", "from __main__ import yajlDec", gettime,10, COUNT)), )




print "\nArray with 256 doubles:"
testObject = []

for x in xrange(256):
   testObject.append(sys.maxint * random.random())
testObjectNP = np.array(testObject)
    
COUNT = 10000

print "ujson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEnc()", "from __main__ import ujsonEnc", gettime,10, COUNT)), )
print "ujson encode NP     : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncNP()", "from __main__ import ujsonEncNP", gettime,10, COUNT)), )
print "ujson encode NP CONV: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncNPConv()", "from __main__ import ujsonEncNPConv", gettime,10, COUNT)), )
print "simplejson encode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEnc()", "from __main__ import simplejsonEnc", gettime,10, COUNT)), )
print "simplejson enc CONV : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEncNP()", "from __main__ import simplejsonEncNP", gettime,10, COUNT)), )
print "cjson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEnc()", "from __main__ import cjsonEnc", gettime, 10, COUNT)), )
print "cjson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEncNP()", "from __main__ import cjsonEncNP", gettime, 10, COUNT)), )
print "yajl  encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEnc()", "from __main__ import yajlEnc", gettime, 10, COUNT)), )
print "yajl  encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEncNP()", "from __main__ import yajlEncNP", gettime, 10, COUNT)), )


print 
decodeData = ujson.dumps(testObject)

print "ujson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDec()", "from __main__ import ujsonDec", gettime,10, COUNT)), )
print "ujson decode NP     : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecNP()", "from __main__ import ujsonDecNP", gettime,10, COUNT)), )
print "ujson decode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecNPConv()", "from __main__ import ujsonDecNPConv", gettime,10, COUNT)), )
print "cjson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonDec()", "from __main__ import cjsonDec", gettime,10, COUNT)), )
print "simplejson decode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonDec()", "from __main__ import simplejsonDec", gettime,10, COUNT)), )
print "yajl decode         : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlDec()", "from __main__ import yajlDec", gettime,10, COUNT)), )




print "\nArray with 2000 doubles:"
testObject = []

for x in xrange(2000):
   testObject.append(sys.maxint * random.random())
testObjectNP = np.array(testObject)
    
COUNT = 1000

print "ujson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEnc()", "from __main__ import ujsonEnc", gettime,10, COUNT)), )
print "ujson encode NP     : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncNP()", "from __main__ import ujsonEncNP", gettime,10, COUNT)), )
print "ujson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncNPConv()", "from __main__ import ujsonEncNPConv", gettime,10, COUNT)), )
print "simplejson encode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEnc()", "from __main__ import simplejsonEnc", gettime,10, COUNT)), )
print "simplejson enc CONV : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEncNP()", "from __main__ import simplejsonEncNP", gettime,10, COUNT)), )
print "cjson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEnc()", "from __main__ import cjsonEnc", gettime, 10, COUNT)), )
print "cjson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEncNP()", "from __main__ import cjsonEncNP", gettime, 10, COUNT)), )
print "yajl  encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEnc()", "from __main__ import yajlEnc", gettime, 10, COUNT)), )
print "yajl  encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEncNP()", "from __main__ import yajlEncNP", gettime, 10, COUNT)), )


print 
decodeData = ujson.dumps(testObject)

print "ujson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDec()", "from __main__ import ujsonDec", gettime,10, COUNT)), )
print "ujson decode NP     : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecNP()", "from __main__ import ujsonDecNP", gettime,10, COUNT)), )
print "ujson decode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecNPConv()", "from __main__ import ujsonDecNPConv", gettime,10, COUNT)), )
print "cjson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonDec()", "from __main__ import cjsonDec", gettime,10, COUNT)), )
print "simplejson decode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonDec()", "from __main__ import simplejsonDec", gettime,10, COUNT)), )
print "yajl decode         : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlDec()", "from __main__ import yajlDec", gettime,10, COUNT)), )




print "\nDictionary/DataFrame 4x200 zeroes :"

df = DataFrame(np.zeros((200, 4)),
        columns=[str(i) for i in range(4)],
        index=[str(i) for i in range(200)])
testObject = df.to_dict()
testObjectDF = df

COUNT = 1000

print "ujson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEnc()", "from __main__ import ujsonEnc", gettime,10, COUNT)), )
print "ujson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDFConv()", "from __main__ import ujsonEncDFConv", gettime,10, COUNT)), )
print "ujson encode VALS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='values')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode COLS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF()", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode SPLT   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='split')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode RECS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='records')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode IDX    : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='index')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "simplejson encode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEnc()", "from __main__ import simplejsonEnc", gettime,10, COUNT)), )
print "simplejson enc CONV : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEncDF()", "from __main__ import simplejsonEncDF", gettime,10, COUNT)), )
print "cjson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEnc()", "from __main__ import cjsonEnc", gettime, 10, COUNT)), )
print "cjson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEncDF()", "from __main__ import cjsonEncDF", gettime, 10, COUNT)), )
print "yajl  encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEnc()", "from __main__ import yajlEnc", gettime, 10, COUNT)), )
print "yajl  encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEncDF()", "from __main__ import yajlEncDF", gettime, 10, COUNT)), )


print 
decodeData = ujson.dumps(testObjectDF)
decodeDataVals = ujson.dumps(testObjectDF, orient="values")
decodeDataSplit = ujson.dumps(testObjectDF, orient="split")
decodeDataRec = ujson.dumps(testObjectDF, orient="records")
decodeDataIdx = ujson.dumps(testObjectDF, orient="index")

print "ujson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDec()", "from __main__ import ujsonDec", gettime,10, COUNT)), )
print "ujson decode VALS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFVal()", "from __main__ import ujsonDecDFVal", gettime,10, COUNT)), )
print "ujson decode VALS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFValNP()", "from __main__ import ujsonDecDFValNP", gettime,10, COUNT)), )
print "ujson decode COLS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDF()", "from __main__ import ujsonDecDF", gettime,10, COUNT)), )
print "ujson decode COLS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFNP()", "from __main__ import ujsonDecDFNP", gettime,10, COUNT)), )
print "ujson decode SPLT   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFSplit()", "from __main__ import ujsonDecDFSplit", gettime,10, COUNT)), )
print "ujson decode SPLT NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFSplitNP()", "from __main__ import ujsonDecDFSplitNP", gettime,10, COUNT)), )
print "ujson decode RECS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFRec()", "from __main__ import ujsonDecDFRec", gettime,10, COUNT)), )
print "ujson decode RECS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFRecNP()", "from __main__ import ujsonDecDFRecNP", gettime,10, COUNT)), )
print "ujson decode IDX    : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFIndex()", "from __main__ import ujsonDecDFIndex", gettime,10, COUNT)), )
print "ujson decode IDX NP : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFIndexNP()", "from __main__ import ujsonDecDFIndexNP", gettime,10, COUNT)), )
print "cjson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonDec()", "from __main__ import cjsonDec", gettime,10, COUNT)), )
print "simplejson decode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonDec()", "from __main__ import simplejsonDec", gettime,10, COUNT)), )
print "yajl decode         : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlDec()", "from __main__ import yajlDec", gettime,10, COUNT)), )





print "\nDictionary/DataFrame 200x4 zeroes :"

df = df.T
testObject = df.to_dict()
testObjectDF = df

COUNT = 1000

print "ujson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEnc()", "from __main__ import ujsonEnc", gettime,10, COUNT)), )
print "ujson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDFConv()", "from __main__ import ujsonEncDFConv", gettime,10, COUNT)), )
print "ujson encode VALS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='values')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode COLS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF()", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode SPLT   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='split')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode RECS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='records')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode IDX    : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='index')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "simplejson encode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEnc()", "from __main__ import simplejsonEnc", gettime,10, COUNT)), )
print "simplejson enc CONV : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEncDF()", "from __main__ import simplejsonEncDF", gettime,10, COUNT)), )
print "cjson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEnc()", "from __main__ import cjsonEnc", gettime, 10, COUNT)), )
print "cjson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEncDF()", "from __main__ import cjsonEncDF", gettime, 10, COUNT)), )
print "yajl  encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEnc()", "from __main__ import yajlEnc", gettime, 10, COUNT)), )
print "yajl  encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEncDF()", "from __main__ import yajlEncDF", gettime, 10, COUNT)), )


print 
decodeData = ujson.dumps(testObjectDF)
decodeDataVals = ujson.dumps(testObjectDF, orient="values")
decodeDataSplit = ujson.dumps(testObjectDF, orient="split")
decodeDataRec = ujson.dumps(testObjectDF, orient="records")
decodeDataIdx = ujson.dumps(testObjectDF, orient="index")

print "ujson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDec()", "from __main__ import ujsonDec", gettime,10, COUNT)), )
print "ujson decode VALS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFVal()", "from __main__ import ujsonDecDFVal", gettime,10, COUNT)), )
print "ujson decode VALS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFValNP()", "from __main__ import ujsonDecDFValNP", gettime,10, COUNT)), )
print "ujson decode COLS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDF()", "from __main__ import ujsonDecDF", gettime,10, COUNT)), )
print "ujson decode COLS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFNP()", "from __main__ import ujsonDecDFNP", gettime,10, COUNT)), )
print "ujson decode SPLT   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFSplit()", "from __main__ import ujsonDecDFSplit", gettime,10, COUNT)), )
print "ujson decode SPLT NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFSplitNP()", "from __main__ import ujsonDecDFSplitNP", gettime,10, COUNT)), )
print "ujson decode RECS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFRec()", "from __main__ import ujsonDecDFRec", gettime,10, COUNT)), )
print "ujson decode RECS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFRecNP()", "from __main__ import ujsonDecDFRecNP", gettime,10, COUNT)), )
print "ujson decode IDX    : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFIndex()", "from __main__ import ujsonDecDFIndex", gettime,10, COUNT)), )
print "ujson decode IDX NP : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFIndexNP()", "from __main__ import ujsonDecDFIndexNP", gettime,10, COUNT)), )
print "cjson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonDec()", "from __main__ import cjsonDec", gettime,10, COUNT)), )
print "simplejson decode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonDec()", "from __main__ import simplejsonDec", gettime,10, COUNT)), )
print "yajl decode         : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlDec()", "from __main__ import yajlDec", gettime,10, COUNT)), )





print "\nTime Series Small (256 rows, 6 columns) date string index:"

df = DataFrame.from_csv(join(dirname(__file__), "samplesmall.df"))
df.index = [d.isoformat() for d in df.index]
testObject = df.to_dict()
testObjectDF = df

COUNT = 2000

print "ujson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEnc()", "from __main__ import ujsonEnc", gettime,10, COUNT)), )
print "ujson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDFConv()", "from __main__ import ujsonEncDFConv", gettime,10, COUNT)), )
print "ujson encode VALS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='values')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode COLS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF()", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode SPLT   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='split')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode RECS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='records')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode IDX    : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='index')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
#print "simplejson encode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEnc()", "from __main__ import simplejsonEnc", gettime,10, COUNT)), )
#print "simplejson enc CONV : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEncDF()", "from __main__ import simplejsonEncDF", gettime,10, COUNT)), )
#print "cjson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEnc()", "from __main__ import cjsonEnc", gettime, 10, COUNT)), )
#print "cjson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEncDF()", "from __main__ import cjsonEncDF", gettime, 10, COUNT)), )
print "yajl  encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEnc()", "from __main__ import yajlEnc", gettime, 10, COUNT)), )
print "yajl  encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEncDF()", "from __main__ import yajlEncDF", gettime, 10, COUNT)), )


print 
decodeData = ujson.dumps(testObjectDF)
decodeDataVals = ujson.dumps(testObjectDF, orient="values")
decodeDataSplit = ujson.dumps(testObjectDF, orient="split")
decodeDataRec = ujson.dumps(testObjectDF, orient="records")
decodeDataIdx = ujson.dumps(testObjectDF, orient="index")

print "ujson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDec()", "from __main__ import ujsonDec", gettime,10, COUNT)), )
print "ujson decode VALS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFVal()", "from __main__ import ujsonDecDFVal", gettime,10, COUNT)), )
print "ujson decode VALS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFValNP()", "from __main__ import ujsonDecDFValNP", gettime,10, COUNT)), )
print "ujson decode COLS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDF()", "from __main__ import ujsonDecDF", gettime,10, COUNT)), )
print "ujson decode COLS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFNP()", "from __main__ import ujsonDecDFNP", gettime,10, COUNT)), )
print "ujson decode SPLT   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFSplit()", "from __main__ import ujsonDecDFSplit", gettime,10, COUNT)), )
print "ujson decode SPLT NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFSplitNP()", "from __main__ import ujsonDecDFSplitNP", gettime,10, COUNT)), )
print "ujson decode RECS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFRec()", "from __main__ import ujsonDecDFRec", gettime,10, COUNT)), )
print "ujson decode RECS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFRecNP()", "from __main__ import ujsonDecDFRecNP", gettime,10, COUNT)), )
print "ujson decode IDX    : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFIndex()", "from __main__ import ujsonDecDFIndex", gettime,10, COUNT)), )
print "ujson decode IDX NP : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFIndexNP()", "from __main__ import ujsonDecDFIndexNP", gettime,10, COUNT)), )
print "cjson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonDec()", "from __main__ import cjsonDec", gettime,10, COUNT)), )
print "simplejson decode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonDec()", "from __main__ import simplejsonDec", gettime,10, COUNT)), )
print "yajl decode         : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlDec()", "from __main__ import yajlDec", gettime,10, COUNT)), )




print "\nTime Series Small (256 rows, 6 columns) Datetime index:"

df = DataFrame.from_csv(join(dirname(__file__), "samplesmall.df"))
testObject = df.to_dict()
testObjectDF = df

COUNT = 2000

print "ujson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEnc()", "from __main__ import ujsonEnc", gettime,10, COUNT)), )
print "ujson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDFConv()", "from __main__ import ujsonEncDFConv", gettime,10, COUNT)), )
print "ujson encode VALS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='values')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode COLS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF()", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode SPLT   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='split')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode RECS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='records')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode IDX    : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='index')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
#print "simplejson encode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEnc()", "from __main__ import simplejsonEnc", gettime,10, COUNT)), )
#print "simplejson enc CONV : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEncDF()", "from __main__ import simplejsonEncDF", gettime,10, COUNT)), )
#print "cjson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEnc()", "from __main__ import cjsonEnc", gettime, 10, COUNT)), )
#print "cjson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEncDF()", "from __main__ import cjsonEncDF", gettime, 10, COUNT)), )
print "yajl  encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEnc()", "from __main__ import yajlEnc", gettime, 10, COUNT)), )
print "yajl  encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEncDF()", "from __main__ import yajlEncDF", gettime, 10, COUNT)), )


print 
decodeData = ujson.dumps(testObjectDF)
decodeDataVals = ujson.dumps(testObjectDF, orient="values")
decodeDataSplit = ujson.dumps(testObjectDF, orient="split")
decodeDataRec = ujson.dumps(testObjectDF, orient="records")
decodeDataIdx = ujson.dumps(testObjectDF, orient="index")

print "ujson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDec()", "from __main__ import ujsonDec", gettime,10, COUNT)), )
print "ujson decode VALS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFVal()", "from __main__ import ujsonDecDFVal", gettime,10, COUNT)), )
print "ujson decode VALS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFValNP()", "from __main__ import ujsonDecDFValNP", gettime,10, COUNT)), )
print "ujson decode COLS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDF()", "from __main__ import ujsonDecDF", gettime,10, COUNT)), )
print "ujson decode COLS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFNP()", "from __main__ import ujsonDecDFNP", gettime,10, COUNT)), )
print "ujson decode SPLT   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFSplit()", "from __main__ import ujsonDecDFSplit", gettime,10, COUNT)), )
print "ujson decode SPLT NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFSplitNP()", "from __main__ import ujsonDecDFSplitNP", gettime,10, COUNT)), )
print "ujson decode RECS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFRec()", "from __main__ import ujsonDecDFRec", gettime,10, COUNT)), )
print "ujson decode RECS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFRecNP()", "from __main__ import ujsonDecDFRecNP", gettime,10, COUNT)), )
print "ujson decode IDX    : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFIndex()", "from __main__ import ujsonDecDFIndex", gettime,10, COUNT)), )
print "ujson decode IDX NP : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFIndexNP()", "from __main__ import ujsonDecDFIndexNP", gettime,10, COUNT)), )
print "cjson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonDec()", "from __main__ import cjsonDec", gettime,10, COUNT)), )
print "simplejson decode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonDec()", "from __main__ import simplejsonDec", gettime,10, COUNT)), )
print "yajl decode         : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlDec()", "from __main__ import yajlDec", gettime,10, COUNT)), )





print "\nTime Series Large (1857 rows, 6 columns) Datetime index:"

df = DataFrame.from_csv(join(dirname(__file__), "sample.df"))
testObject = df.to_dict()
testObjectDF = df

COUNT = 100

print "ujson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEnc()", "from __main__ import ujsonEnc", gettime,10, COUNT)), )
print "ujson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDFConv()", "from __main__ import ujsonEncDFConv", gettime,10, COUNT)), )
print "ujson encode VALS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='values')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode COLS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF()", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode SPLT   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='split')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode RECS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='records')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "ujson encode IDX    : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonEncDF(orient='index')", "from __main__ import ujsonEncDF", gettime,10, COUNT)), )
print "simplejson encode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEnc()", "from __main__ import simplejsonEnc", gettime,10, COUNT)), )
print "simplejson enc CONV : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonEncDF()", "from __main__ import simplejsonEncDF", gettime,10, COUNT)), )
print "cjson encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEnc()", "from __main__ import cjsonEnc", gettime, 10, COUNT)), )
print "cjson encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonEncDF()", "from __main__ import cjsonEncDF", gettime, 10, COUNT)), )
print "yajl  encode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEnc()", "from __main__ import yajlEnc", gettime, 10, COUNT)), )
print "yajl  encode CONV   : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlEncDF()", "from __main__ import yajlEncDF", gettime, 10, COUNT)), )


print 
decodeData = ujson.dumps(testObjectDF)
decodeDataVals = ujson.dumps(testObjectDF, orient="values")
decodeDataSplit = ujson.dumps(testObjectDF, orient="split")
decodeDataRec = ujson.dumps(testObjectDF, orient="records")
decodeDataIdx = ujson.dumps(testObjectDF, orient="index")

print "ujson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDec()", "from __main__ import ujsonDec", gettime,10, COUNT)), )
print "ujson decode VALS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFVal()", "from __main__ import ujsonDecDFVal", gettime,10, COUNT)), )
print "ujson decode VALS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFValNP()", "from __main__ import ujsonDecDFValNP", gettime,10, COUNT)), )
print "ujson decode COLS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDF()", "from __main__ import ujsonDecDF", gettime,10, COUNT)), )
print "ujson decode COLS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFNP()", "from __main__ import ujsonDecDFNP", gettime,10, COUNT)), )
print "ujson decode SPLT   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFSplit()", "from __main__ import ujsonDecDFSplit", gettime,10, COUNT)), )
print "ujson decode SPLT NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFSplitNP()", "from __main__ import ujsonDecDFSplitNP", gettime,10, COUNT)), )
print "ujson decode RECS   : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFRec()", "from __main__ import ujsonDecDFRec", gettime,10, COUNT)), )
print "ujson decode RECS NP: %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFRecNP()", "from __main__ import ujsonDecDFRecNP", gettime,10, COUNT)), )
print "ujson decode IDX    : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFIndex()", "from __main__ import ujsonDecDFIndex", gettime,10, COUNT)), )
print "ujson decode IDX NP : %.05f calls/sec" % (COUNT / min(timeit.repeat("ujsonDecDFIndexNP()", "from __main__ import ujsonDecDFIndexNP", gettime,10, COUNT)), )
print "cjson decode        : %.05f calls/sec" % (COUNT / min(timeit.repeat("cjsonDec()", "from __main__ import cjsonDec", gettime,10, COUNT)), )
print "simplejson decode   : %.05f calls/sec" % (COUNT / min(timeit.repeat("simplejsonDec()", "from __main__ import simplejsonDec", gettime,10, COUNT)), )
print "yajl decode         : %.05f calls/sec" % (COUNT / min(timeit.repeat("yajlDec()", "from __main__ import yajlDec", gettime,10, COUNT)), )
