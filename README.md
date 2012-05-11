A fork of UltraJSON supporting direct encoding / decoding of numpy and pandas
types.

UltraJSON is a fast and extendable JSON encoder and decoder written in pure C.

When encoding, a range of DataFrame output formats are offered (see below).

When decoding to numpy arrays, the dtype is 'sniffed' from the first decoded
array entry, or the desired dtype can be specified beforehand. Note decoding
to numpy is switched off by default. 

With the labelled option set to true JSON objects are decoded to numpy arrays 
of keys and values, returned in a tuple. This is to support DataFrame 
decoding (see example below).

Datetime handling has also been improved and maximum double precision and max 
floating point values have been increased.

Benchmarks:

64-bit benchmarks MAC OSX (Snow Leopard)  
Python 2.7.2+ (default, Feb 20 2012, 23:30:20)  
OS Version: Mac OSX 10.6.8  
System Type: x64-based MacBook Pro  
Processor: Intel(R) Core 2 Duo(TM) CPU @ 2.4 GHz  
Total Physical Memory: 4096 MB  

numpy 1.6.1  
pandas 0.8.0.dev-4c4c7b3

py-yajl 0.3.5 => http://rtyler.github.com/py-yajl  
simplejson 2.3.2 => http://simplejson.readthedocs.org/en/latest/index.html  
cjson 1.0.5 => http://pypi.python.org/pypi/python-cjson

Note the main interest when benchmarking was to get a rough idea of the 
likely performance improvement if numpy and pandas data types
are dealt with directly, rather than converted, when encoding and decoding.

Legend:

 - NP: Direct encoding/decoding from/to numpy or pandas data type.
 - (NP) CONV: Numpy / pandas type converted before/after encoding/decoding.
 - encode (VALS | SPLT | RECS | COLS | IDX): DataFrame encoded directly
 (without pre-conversion) in specified format.
 - decode (VALS | SPLT | RECS | COLS | IDX): decoded to Python list or dict 
 from the specified format before creating DataFrame.
 - decode (VALS | SPLT | RECS | COLS | IDX) NP: decoded to numpy array(s) 
 from the specified format before creating DataFrame.
 - If none of the above are specifed then only the equivalent basic Python 
 types were involved.

DataFrame formats:
 - SPLT: DataFrame format option 'split', dict like
`{index -> [index], columns -> [columns], data -> [values]}`.
 - RECS: DataFrame format option 'records', list like
 `[{column -> value}, ... , {column -> value}]`.
 - COLS: DataFrame format option 'columns', dict like
`{column -> {index -> value}}`.
 - IDX:  DataFrame format option 'index', dict like
 `{index -> {column -> value}}`.
 - VALS: DataFrame format option 'values', values array only.

```python
In [1]: import numpy as np

In [2]: from pandas import DataFrame

In [3]: import ujson

In [4]: npenc = ujson.dumps(np.array([10,11,12,13,14,15]).reshape((3,2)))

In [5]: npenc
Out[5]: '[[10,11],[12,13],[14,15]]'

In [6]: ujson.loads(npenc)
Out[6]: [[10, 11], [12, 13], [14, 15]]

In [7]: ujson.loads(npenc, numpy=True)
Out[7]: 
array([[10, 11],
       [12, 13],
       [14, 15]])

In [8]: ujson.loads(npenc, numpy=True, dtype=np.float64)
Out[8]: 
array([[ 10.,  11.],
       [ 12.,  13.],
       [ 14.,  15.]])

In [9]: df = DataFrame([[1,2,3],[4,5,6]], ['a','b'], ['x','y','z'])

In [10]: df
Out[10]: 
   x  y  z
a  1  2  3
b  4  5  6

In [11]: ujson.dumps(df, orient='split')
Out[11]: '{"columns":["x","y","z"],"index":["a","b"],"data":[[1,2,3],[4,5,6]]}'

In [12]: ujson.dumps(df, orient='records')
Out[12]: '[{"x":1,"y":2,"z":3},{"x":4,"y":5,"z":6}]'

In [13]: ujson.dumps(df, orient='columns')
Out[13]: '{"x":{"a":1,"b":4},"y":{"a":2,"b":5},"z":{"a":3,"b":6}}'

In [14]: ujson.dumps(df, orient='index')
Out[14]: '{"a":{"x":1,"y":2,"z":3},"b":{"x":4,"y":5,"z":6}}'

In [15]: ujson.dumps(df, orient='values')
Out[15]: '[[1,2,3],[4,5,6]]'

In [16]: dfenc = ujson.dumps(df, orient='index')

In [17]: ujson.loads(dfenc)
Out[17]: {u'a': {u'x': 1, u'y': 2, u'z': 3}, u'b': {u'x': 4, u'y': 5, u'z': 6}}

In [18]: ujson.loads(dfenc, numpy=True)
Out[18]: {u'a': {u'x': 1, u'y': 2, u'z': 3}, u'b': {u'x': 4, u'y': 5, u'z': 6}}

In [19]: ujson.loads(dfenc, numpy=True, labelled=True)
Out[19]: 
(array([[1, 2, 3],
       [4, 5, 6]]),
 array([u'a', u'b'], 
      dtype='<U1'),
 array([u'x', u'y', u'z'], 
      dtype='<U1'))
```
```
Array with 256 utf-8 strings :
ujson encode        : 4923.96119 calls/sec
ujson encode NP     : 2937.75959 calls/sec
ujson encode NP CONV: 3717.10569 calls/sec
simplejson encode   : 869.09586 calls/sec
simplejson enc CONV : 814.62940 calls/sec
cjson encode        : 70.14297 calls/sec
cjson encode CONV   : 69.75142 calls/sec
yajl  encode        : 3473.24166 calls/sec
yajl  encode CONV   : 2881.26728 calls/sec

ujson decode        : 1005.14133 calls/sec
ujson decode NP     : 775.17548 calls/sec
ujson decode CONV   : 794.24993 calls/sec
simplejson decode   : 194.93789 calls/sec
cjson decode        : 648.48635 calls/sec
yajl decode         : 404.25102 calls/sec

Array with 256 doubles:
ujson encode        : 3087.53188 calls/sec
ujson encode NP     : 2964.00633 calls/sec
ujson encode NP CONV: 3046.01775 calls/sec
simplejson encode   : 1774.49091 calls/sec
simplejson enc CONV : 1756.58724 calls/sec
cjson encode        : 5861.13280 calls/sec
cjson encode CONV   : 5677.19922 calls/sec
yajl  encode        : 7280.77035 calls/sec
yajl  encode CONV   : 6995.48279 calls/sec

ujson decode        : 27652.33891 calls/sec
ujson decode NP     : 22364.71899 calls/sec
ujson decode CONV   : 12499.09184 calls/sec
cjson decode        : 6517.84250 calls/sec
simplejson decode   : 8341.36252 calls/sec
yajl decode         : 7721.88216 calls/sec

Array with 2000 doubles:
ujson encode        : 399.18741 calls/sec
ujson encode NP     : 386.76147 calls/sec
ujson encode CONV   : 398.13152 calls/sec
simplejson encode   : 237.89947 calls/sec
simplejson enc CONV : 234.95865 calls/sec
cjson encode        : 780.36975 calls/sec
cjson encode CONV   : 756.81099 calls/sec
yajl  encode        : 959.16262 calls/sec
yajl  encode CONV   : 924.05212 calls/sec

ujson decode        : 3942.54862 calls/sec
ujson decode NP     : 3209.14962 calls/sec
ujson decode CONV   : 1990.46979 calls/sec
cjson decode        : 829.26242 calls/sec
simplejson decode   : 1173.59890 calls/sec
yajl decode         : 1022.19818 calls/sec

Dictionary/DataFrame 4x200 zeroes :
ujson encode        : 7989.13526 calls/sec
ujson encode CONV   : 1956.59875 calls/sec
ujson encode VALS   : 6887.15345 calls/sec
ujson encode COLS   : 4947.60682 calls/sec
ujson encode SPLT   : 5640.91722 calls/sec
ujson encode RECS   : 6138.80839 calls/sec
ujson encode IDX    : 4534.03959 calls/sec
simplejson encode   : 1134.72856 calls/sec
simplejson enc CONV : 788.82455 calls/sec
cjson encode        : 1189.99163 calls/sec
cjson encode CONV   : 810.34837 calls/sec
yajl  encode        : 2957.14634 calls/sec
yajl  encode CONV   : 1379.58999 calls/sec

ujson decode        : 7388.57866 calls/sec
ujson decode VALS   : 1502.10347 calls/sec
ujson decode VALS NP: 4870.82224 calls/sec
ujson decode COLS   : 991.17741 calls/sec
ujson decode COLS NP: 2398.96178 calls/sec*
ujson decode SPLT   : 1429.35173 calls/sec
ujson decode SPLT NP: 2522.93363 calls/sec
ujson decode RECS   : 981.51038 calls/sec
ujson decode RECS NP: 3194.94939 calls/sec
ujson decode IDX    : 113.65474 calls/sec*
ujson decode IDX NP : 2226.16021 calls/sec
cjson decode        : 2910.96251 calls/sec
simplejson decode   : 4076.72315 calls/sec
yajl decode         : 3126.63132 calls/sec

Dictionary/DataFrame 200x4 zeroes :
ujson encode        : 6211.45556 calls/sec
ujson encode CONV   : 247.13131 calls/sec
ujson encode VALS   : 7939.02123 calls/sec
ujson encode COLS   : 4507.08197 calls/sec
ujson encode SPLT   : 6315.56250 calls/sec
ujson encode RECS   : 4977.32715 calls/sec
ujson encode IDX    : 4930.35700 calls/sec
simplejson encode   : 744.41256 calls/sec
simplejson enc CONV : 190.22875 calls/sec
cjson encode        : 962.46940 calls/sec
cjson encode CONV   : 203.07179 calls/sec
yajl  encode        : 2704.39739 calls/sec
yajl  encode CONV   : 234.48830 calls/sec

ujson decode        : 5615.92483 calls/sec
ujson decode VALS   : 129.64719 calls/sec
ujson decode VALS NP: 4860.00853 calls/sec
ujson decode COLS   : 115.31494 calls/sec
ujson decode COLS NP: 1899.72498 calls/sec*
ujson decode SPLT   : 150.52291 calls/sec
ujson decode SPLT NP: 2177.77995 calls/sec
ujson decode RECS   : 138.80699 calls/sec
ujson decode RECS NP: 2131.71455 calls/sec
ujson decode IDX    : 824.87504 calls/sec*
ujson decode IDX NP : 2047.83301 calls/sec
cjson decode        : 2530.46046 calls/sec
simplejson decode   : 3851.02728 calls/sec
yajl decode         : 3078.43796 calls/sec
      
Time Series Small (256 rows, 6 columns) date string index:
ujson encode        : 2341.75276 calls/sec
ujson encode CONV   : 892.56349 calls/sec
ujson encode VALS   : 1970.84313 calls/sec
ujson encode COLS   : 1386.16186 calls/sec
ujson encode SPLT   : 1778.11467 calls/sec
ujson encode RECS   : 1724.85038 calls/sec
ujson encode IDX    : 1302.95115 calls/sec
yajl  encode        : 1077.76857 calls/sec
yajl  encode CONV   : 620.21026 calls/sec

ujson decode        : 1755.62721 calls/sec
ujson decode VALS   : 1105.68957 calls/sec
ujson decode VALS NP: 3553.78115 calls/sec
ujson decode COLS   : 524.17068 calls/sec
ujson decode COLS NP: 1266.49862 calls/sec*
ujson decode SPLT   : 1036.01228 calls/sec
ujson decode SPLT NP: 1774.22301 calls/sec
ujson decode RECS   : 549.63475 calls/sec
ujson decode RECS NP: 1839.64373 calls/sec
ujson decode IDX    : 82.87102 calls/sec*
ujson decode IDX NP : 1266.69330 calls/sec
cjson decode        : 1136.14725 calls/sec
simplejson decode   : 1369.50672 calls/sec
yajl decode         : 1081.40544 calls/sec

Time Series Small (256 rows, 6 columns) Datetime index:
ujson encode        : 368.02051 calls/sec
ujson encode CONV   : 85.00741 calls/sec
ujson encode VALS   : 1970.88712 calls/sec
ujson encode COLS   : 597.94299 calls/sec
ujson encode SPLT   : 641.48194 calls/sec
ujson encode RECS   : 1740.32817 calls/sec
ujson encode IDX    : 595.10556 calls/sec
yajl  encode        : 5692.47912 calls/sec
yajl  encode CONV   : 106.67600 calls/sec
                
ujson decode        : 2279.19656 calls/sec
ujson decode VALS   : 1111.16471 calls/sec
ujson decode VALS NP: 3594.45274 calls/sec
ujson decode COLS   : 575.44799 calls/sec
ujson decode COLS NP: 1465.78544 calls/sec*
ujson decode SPLT   : 1040.64821 calls/sec
ujson decode SPLT NP: 2628.88467 calls/sec
ujson decode RECS   : 559.05698 calls/sec
ujson decode RECS NP: 1847.11457 calls/sec
ujson decode IDX    : 84.82150 calls/sec*
ujson decode IDX NP : 1324.37263 calls/sec
cjson decode        : 1236.42332 calls/sec
simplejson decode   : 1570.85507 calls/sec
yajl decode         : 1255.85240 calls/sec

Time Series Large (1857 rows, 6 columns) Datetime index:
ujson encode        : 45.77206 calls/sec
ujson encode CONV   : 11.49803 calls/sec
ujson encode VALS   : 378.06448 calls/sec
ujson encode COLS   : 88.11304 calls/sec
ujson encode SPLT   : 95.21860 calls/sec
ujson encode RECS   : 250.60714 calls/sec
ujson encode IDX    : 78.70242 calls/sec
yajl  encode        : 610.23591 calls/sec
yajl  encode CONV   : 14.37145 calls/sec

ujson decode        : 256.36364 calls/sec
ujson decode VALS   : 362.10736 calls/sec
ujson decode VALS NP: 851.45553 calls/sec
ujson decode COLS   : 91.49072 calls/sec
ujson decode COLS NP: 279.50306 calls/sec*
ujson decode SPLT   : 324.54158 calls/sec
ujson decode SPLT NP: 658.90470 calls/sec
ujson decode RECS   : 83.18138 calls/sec
ujson decode RECS NP: 372.51444 calls/sec
*ujson decode IDX   : 11.18950 calls/sec*
ujson decode IDX NP : 243.83872 calls/sec
cjson decode        : 158.10552 calls/sec
simplejson decode   : 200.60099 calls/sec
yajl decode         : 154.91891 calls/sec   

*) Transpose required after decoding
```
See (python/pandasbenchmark.py) and README_UJSON for further information.


