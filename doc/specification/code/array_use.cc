MHO_NDArrayWrapper< double, 3> ex;
//declare the dimensions of the 3d array
ex.Resize(10,10,10);

//access via un-checked index tuples
ex(0,0,0) = 1;
//access via bounds-checked index tuples
ex.at(9,9,9) = 1;
//access via un-checked offset from start 
ex[999] = 1;
//access to underlying raw memory
double* ptr = ex.GetData();
ptr[3] = 1;
//access via iterator 
auto it = ex.begin();
*it = 1;
//skip-by-10 access via strided iterator 
auto sit = ex.stride_begin(10)
*(++sit) = 1;  //access to ex(0,1,0);

