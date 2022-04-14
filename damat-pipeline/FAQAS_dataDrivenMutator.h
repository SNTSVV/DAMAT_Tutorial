//max MUTATIONOPT=42//
// Copyright (c) University of Luxembourg 2020.
// Created by Fabrizio PASTORE, fabrizio.pastore@uni.lu, SnT, 2020.
// Modified by Oscar Eduardo CORNEJO OLIVARES, oscar.cornejo@uni.lu, SnT, 2020.
// Modified by Enrico VIGANO', enrico.vigano@uni.lu, SnT, 2021.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef __cplusplus
#include <vector>
#endif

#define MAX_OPS 200
#define ITEMS 10

//macros to define at compilation
int MUTATION = MUTATIONOPT;

int _FAQAS_COVERAGE_EXIT = 0;

int global_mutation_counter = 1;

#ifdef _FAQAS_MUTATION_PROBABILITY
float PROBABILITY = _FAQAS_MUTATION_PROBABILITY;
#endif


//this handles coverage without continuosly opening and closing files for g++

#ifdef __cplusplus

FILE* handleCoverage();

const char *faqas_coverage_file = getenv("FAQAS_COVERAGE_FILE");

FILE *coverage_file_pointer = handleCoverage(); // this is the problem

void coverage_exit(void) {

fclose(coverage_file_pointer);

}

FILE* handleCoverage() {

   FILE* ptr = fopen(faqas_coverage_file, "ab+");

    if (_FAQAS_COVERAGE_EXIT == 0) {
        _FAQAS_COVERAGE_EXIT = 1;
        // if there is no support for atexit, or if the system never exit,
        // this line of code should be commented out and "coverage_exit"
        // shall be manually invoked where appropriate
        atexit(coverage_exit);
    }

    return ptr;
}

#endif



double faqas_abs(double a) {
  if (a < 0)
    return -a;
  return a;
}

int faqas_double_equal(double a, double b) { return faqas_abs(a - b) < 1E-3; }

unsigned long long int FAQAS_pow_substitute(int base, int power){
  //this function substitutes "pow" so that including math.h is not needed
  if(power < 0){
    return 0;
  }

  unsigned long long int result=1;

  if(power >= 0){
    int step;
    for ( step=1; step <= power; step=step+1 ) {
      result= result * base;
    }
  }
  return result;
}


enum DataType { INT, FLOAT, DOUBLE, BIN, LONG };

typedef enum DataType DataType;

enum MutationType { BF, IV, VOR, FVOR, VAT, FVAT, VBT, FVBT, INV, SS, ASA, HV };

typedef enum MutationType MutationType;

int _FAQAS_mutated = 0;

struct MutationOperator {
  MutationType type;

  double min;
  double max;
  double threshold;
  double delta;
  int state;
  double value;
};

typedef struct MutationOperator MutationOperator;

struct DataItem {
  DataType type;
  int span;
  int operatorsN;
  struct MutationOperator operators[MAX_OPS];
};

struct FaultModel {
  int itemsN;
  int ID;
  int minOperation; // min ID of the MUTATION OPERATION implemented by this FM,
                    // included
  int maxOperation; // max ID of the MUTATION OPERATION implemented by this FM,
                    // included

  struct DataItem *items;
};

typedef struct FaultModel FaultModel;

struct FaultModel *_FAQAS_create_FM(int items) {

  #ifdef __cplusplus

  struct FaultModel *dm = new struct FaultModel;

  dm->itemsN = items;

  dm->items = new DataItem[items];

  #else

  struct FaultModel *dm = (struct FaultModel *) malloc ( sizeof( *dm ) );

  dm->items = (struct DataItem *) malloc( sizeof ( struct DataItem ) * items );

  #endif

  return dm;
}

void __FAQAS_delete_FM(FaultModel *dm) {



  if (dm == 0)
    return;

  #ifdef __cplusplus

  delete[] dm->items;

  dm->items = 0;

  delete dm;

  #else

  free( dm->items );

  free( dm );

  #endif

}


void _FAQAS_delete_FM(FaultModel *dm) {

  #ifdef _FAQAS_SINGLETON_FM
  #else
  __FAQAS_delete_FM(dm);
  #endif
}

// memory for HV
int storedValueInt;
unsigned long long storedValueBin;
double storedValueDouble;
float storedValueFloat;
long int storedValueLong;

int repeatCounter;
int sample = 1;

//_FAQAS_slice_it_up divides long integers in "slices" of binary to be stored in
//the elements of the buffer when span!=1
unsigned long long _FAQAS_slice_it_up(unsigned long long numberToSlice,
                                      int sliceStart, int sliceEnd) {
  int i = sliceStart;
  unsigned long long slice = 0;
  while (i <= sliceEnd) {
    unsigned long long mask = FAQAS_pow_substitute(2, i);
    unsigned long long relevant = numberToSlice & mask;

    if (relevant == mask) {
      unsigned long long knife = FAQAS_pow_substitute(2, i - sliceStart);
      slice = slice | knife;
    }
    i = i + 1;
  }
  return (slice);
}


void _FAQAS_operator_coverage(int operator_id, int counter, int status){

  #ifndef __cplusplus
  char *faqas_coverage_file = getenv("FAQAS_COVERAGE_FILE");
  FILE* coverage_file_pointer = fopen(faqas_coverage_file, "ab+");
  #endif

  fprintf(coverage_file_pointer, "%d,%d,%d \n", operator_id, counter, status);

  #ifndef __cplusplus
  //close the file
  fclose(coverage_file_pointer);
  #endif

}

int _FAQAS_mutate(int *data, FaultModel *fm);
#define SIZE_Read 7
#define SIZE_Send 7


struct FaultModel* _FAQAS_Read_FM_ptr = 0;

void _FAQAS_delete_Read_FM(void){
__FAQAS_delete_FM(_FAQAS_Read_FM_ptr);
_FAQAS_Read_FM_ptr = 0;
}
struct FaultModel* _FAQAS_Read_FM(){
if ( _FAQAS_Read_FM_ptr != 0 ){ return _FAQAS_Read_FM_ptr;}
atexit(_FAQAS_delete_Read_FM);
FaultModel *fm = _FAQAS_create_FM(SIZE_Read);
_FAQAS_Read_FM_ptr = fm;
fm->ID = 0;
fm->minOperation = 0;

fm->items[2].operators[0].type=INV;
fm->items[2].operators[0].min=0;
fm->items[2].operators[0].max=31;

fm->items[2].operators[1].type=VAT;
fm->items[2].operators[1].threshold=31;
fm->items[2].operators[1].delta=1;
fm->items[2].operatorsN=2;
fm->items[2].span=1;
fm->items[2].type=INT;

fm->items[3].operators[0].type=INV;
fm->items[3].operators[0].min=0;
fm->items[3].operators[0].max=31;

fm->items[3].operators[1].type=VAT;
fm->items[3].operators[1].threshold=31;
fm->items[3].operators[1].delta=1;
fm->items[3].operatorsN=2;
fm->items[3].span=1;
fm->items[3].type=INT;

fm->items[4].operators[0].type=INV;
fm->items[4].operators[0].min=8;
fm->items[4].operators[0].max=30;

fm->items[4].operators[1].type=VOR;
fm->items[4].operators[1].min=8;
fm->items[4].operators[1].max=30;
fm->items[4].operators[1].delta=1;

fm->items[4].operators[2].type=FVOR;
fm->items[4].operators[2].min=8;
fm->items[4].operators[2].max=30;

fm->items[4].operators[3].type=SS;
fm->items[4].operators[3].delta=1;

fm->items[4].operators[4].type=SS;
fm->items[4].operators[4].delta=-1;
fm->items[4].operatorsN=5;
fm->items[4].span=1;
fm->items[4].type=INT;

fm->items[5].operators[0].type=INV;
fm->items[5].operators[0].min=8;
fm->items[5].operators[0].max=30;

fm->items[5].operators[1].type=VOR;
fm->items[5].operators[1].min=8;
fm->items[5].operators[1].max=30;
fm->items[5].operators[1].delta=1;

fm->items[5].operators[2].type=FVOR;
fm->items[5].operators[2].min=8;
fm->items[5].operators[2].max=30;

fm->items[5].operators[3].type=SS;
fm->items[5].operators[3].delta=1;

fm->items[5].operators[4].type=SS;
fm->items[5].operators[4].delta=-1;
fm->items[5].operatorsN=5;
fm->items[5].span=1;
fm->items[5].type=INT;

fm->items[6].operators[0].type=BF;
fm->items[6].operators[0].min=0;
fm->items[6].operators[0].max=0;
fm->items[6].operators[0].state=0;
fm->items[6].operators[0].value=1;

fm->items[6].operators[1].type=BF;
fm->items[6].operators[1].min=1;
fm->items[6].operators[1].max=1;
fm->items[6].operators[1].state=0;
fm->items[6].operators[1].value=1;

fm->items[6].operators[2].type=BF;
fm->items[6].operators[2].min=2;
fm->items[6].operators[2].max=2;
fm->items[6].operators[2].state=0;
fm->items[6].operators[2].value=1;

fm->items[6].operators[3].type=BF;
fm->items[6].operators[3].min=3;
fm->items[6].operators[3].max=3;
fm->items[6].operators[3].state=0;
fm->items[6].operators[3].value=1;
fm->items[6].operatorsN=4;
fm->items[6].span=1;
fm->items[6].type=BIN;
fm->maxOperation = 20;
return fm;
}


#ifdef __cplusplus

void mutate_FM_Read(std::vector<int> *v){
    FaultModel *fm = _FAQAS_Read_FM();
    _FAQAS_mutate(v->data(),fm);
    _FAQAS_delete_FM(fm);
}

#else

void mutate_FM_Read( int *v){
    FaultModel *fm = _FAQAS_Read_FM();
    _FAQAS_mutate(v,fm);
    _FAQAS_delete_FM(fm);
}

#endif

struct FaultModel* _FAQAS_Send_FM_ptr = 0;

void _FAQAS_delete_Send_FM(void){
__FAQAS_delete_FM(_FAQAS_Send_FM_ptr);
_FAQAS_Send_FM_ptr = 0;
}
struct FaultModel* _FAQAS_Send_FM(){
if ( _FAQAS_Send_FM_ptr != 0 ){ return _FAQAS_Send_FM_ptr;}
atexit(_FAQAS_delete_Send_FM);
FaultModel *fm = _FAQAS_create_FM(SIZE_Send);
_FAQAS_Send_FM_ptr = fm;
fm->ID = 1;
fm->minOperation = 20;

fm->items[1].operators[3].type=INV;
fm->items[1].operators[3].min=0;
fm->items[1].operators[3].max=3;

fm->items[1].operators[4].type=VAT;
fm->items[1].operators[4].threshold=3;
fm->items[1].operators[4].delta=1;

fm->items[1].operators[5].type=FVAT;
fm->items[1].operators[5].threshold=3;
fm->items[1].operators[5].delta=1;
fm->items[1].operatorsN=6;
fm->items[1].span=1;
fm->items[1].type=INT;

fm->items[2].operators[0].type=INV;
fm->items[2].operators[0].min=0;
fm->items[2].operators[0].max=31;

fm->items[2].operators[1].type=VAT;
fm->items[2].operators[1].threshold=31;
fm->items[2].operators[1].delta=1;
fm->items[2].operatorsN=2;
fm->items[2].span=1;
fm->items[2].type=INT;

fm->items[3].operators[0].type=INV;
fm->items[3].operators[0].min=0;
fm->items[3].operators[0].max=31;

fm->items[3].operators[1].type=VAT;
fm->items[3].operators[1].threshold=31;
fm->items[3].operators[1].delta=1;
fm->items[3].operatorsN=2;
fm->items[3].span=1;
fm->items[3].type=INT;

fm->items[4].operators[0].type=INV;
fm->items[4].operators[0].min=8;
fm->items[4].operators[0].max=30;

fm->items[4].operators[1].type=VOR;
fm->items[4].operators[1].min=8;
fm->items[4].operators[1].max=30;
fm->items[4].operators[1].delta=1;

fm->items[4].operators[2].type=FVOR;
fm->items[4].operators[2].min=8;
fm->items[4].operators[2].max=30;

fm->items[4].operators[3].type=SS;
fm->items[4].operators[3].delta=1;

fm->items[4].operators[4].type=SS;
fm->items[4].operators[4].delta=-1;
fm->items[4].operatorsN=5;
fm->items[4].span=1;
fm->items[4].type=INT;

fm->items[5].operators[0].type=INV;
fm->items[5].operators[0].min=8;
fm->items[5].operators[0].max=30;

fm->items[5].operators[1].type=VOR;
fm->items[5].operators[1].min=8;
fm->items[5].operators[1].max=30;
fm->items[5].operators[1].delta=1;

fm->items[5].operators[2].type=FVOR;
fm->items[5].operators[2].min=8;
fm->items[5].operators[2].max=30;

fm->items[5].operators[3].type=SS;
fm->items[5].operators[3].delta=1;

fm->items[5].operators[4].type=SS;
fm->items[5].operators[4].delta=-1;
fm->items[5].operatorsN=5;
fm->items[5].span=1;
fm->items[5].type=INT;

fm->items[6].operators[0].type=BF;
fm->items[6].operators[0].min=0;
fm->items[6].operators[0].max=0;
fm->items[6].operators[0].state=0;
fm->items[6].operators[0].value=1;

fm->items[6].operators[1].type=BF;
fm->items[6].operators[1].min=1;
fm->items[6].operators[1].max=1;
fm->items[6].operators[1].state=0;
fm->items[6].operators[1].value=1;

fm->items[6].operators[2].type=BF;
fm->items[6].operators[2].min=2;
fm->items[6].operators[2].max=2;
fm->items[6].operators[2].state=0;
fm->items[6].operators[2].value=1;

fm->items[6].operators[3].type=BF;
fm->items[6].operators[3].min=3;
fm->items[6].operators[3].max=3;
fm->items[6].operators[3].state=0;
fm->items[6].operators[3].value=1;
fm->items[6].operatorsN=4;
fm->items[6].span=1;
fm->items[6].type=BIN;
fm->maxOperation = 43;
return fm;
}


#ifdef __cplusplus

void mutate_FM_Send(std::vector<int> *v){
    FaultModel *fm = _FAQAS_Send_FM();
    _FAQAS_mutate(v->data(),fm);
    _FAQAS_delete_FM(fm);
}

#else

void mutate_FM_Send( int *v){
    FaultModel *fm = _FAQAS_Send_FM();
    _FAQAS_mutate(v,fm);
    _FAQAS_delete_FM(fm);
}

#endif



int _FAQAS_selectItem(){
if ( MUTATION == 0 )
    return 2;
if ( MUTATION == 1 )
    return 2;
if ( MUTATION == 2 )
    return 3;
if ( MUTATION == 3 )
    return 3;
if ( MUTATION == 4 )
    return 4;
if ( MUTATION == 5 )
    return 4;
if ( MUTATION == 6 )
    return 4;
if ( MUTATION == 7 )
    return 4;
if ( MUTATION == 8 )
    return 4;
if ( MUTATION == 9 )
    return 4;
if ( MUTATION == 10 )
    return 5;
if ( MUTATION == 11 )
    return 5;
if ( MUTATION == 12 )
    return 5;
if ( MUTATION == 13 )
    return 5;
if ( MUTATION == 14 )
    return 5;
if ( MUTATION == 15 )
    return 5;
if ( MUTATION == 16 )
    return 6;
if ( MUTATION == 17 )
    return 6;
if ( MUTATION == 18 )
    return 6;
if ( MUTATION == 19 )
    return 6;
if ( MUTATION == 20 )
    return 1;
if ( MUTATION == 21 )
    return 1;
if ( MUTATION == 22 )
    return 1;
if ( MUTATION == 23 )
    return 2;
if ( MUTATION == 24 )
    return 2;
if ( MUTATION == 25 )
    return 3;
if ( MUTATION == 26 )
    return 3;
if ( MUTATION == 27 )
    return 4;
if ( MUTATION == 28 )
    return 4;
if ( MUTATION == 29 )
    return 4;
if ( MUTATION == 30 )
    return 4;
if ( MUTATION == 31 )
    return 4;
if ( MUTATION == 32 )
    return 4;
if ( MUTATION == 33 )
    return 5;
if ( MUTATION == 34 )
    return 5;
if ( MUTATION == 35 )
    return 5;
if ( MUTATION == 36 )
    return 5;
if ( MUTATION == 37 )
    return 5;
if ( MUTATION == 38 )
    return 5;
if ( MUTATION == 39 )
    return 6;
if ( MUTATION == 40 )
    return 6;
if ( MUTATION == 41 )
    return 6;
if ( MUTATION == 42 )
    return 6;
return -999;
}
int _FAQAS_INITIAL_PADDING =0; 
int _FAQAS_selectOperator(){
if ( MUTATION == 0 )
    return 0;
if ( MUTATION == 1 )
    return 1;
if ( MUTATION == 2 )
    return 0;
if ( MUTATION == 3 )
    return 1;
if ( MUTATION == 4 )
    return 0;
if ( MUTATION == 5 )
    return 1;
if ( MUTATION == 6 )
    return 1;
if ( MUTATION == 7 )
    return 2;
if ( MUTATION == 8 )
    return 3;
if ( MUTATION == 9 )
    return 4;
if ( MUTATION == 10 )
    return 0;
if ( MUTATION == 11 )
    return 1;
if ( MUTATION == 12 )
    return 1;
if ( MUTATION == 13 )
    return 2;
if ( MUTATION == 14 )
    return 3;
if ( MUTATION == 15 )
    return 4;
if ( MUTATION == 16 )
    return 0;
if ( MUTATION == 17 )
    return 1;
if ( MUTATION == 18 )
    return 2;
if ( MUTATION == 19 )
    return 3;
if ( MUTATION == 20 )
    return 3;
if ( MUTATION == 21 )
    return 4;
if ( MUTATION == 22 )
    return 5;
if ( MUTATION == 23 )
    return 0;
if ( MUTATION == 24 )
    return 1;
if ( MUTATION == 25 )
    return 0;
if ( MUTATION == 26 )
    return 1;
if ( MUTATION == 27 )
    return 0;
if ( MUTATION == 28 )
    return 1;
if ( MUTATION == 29 )
    return 1;
if ( MUTATION == 30 )
    return 2;
if ( MUTATION == 31 )
    return 3;
if ( MUTATION == 32 )
    return 4;
if ( MUTATION == 33 )
    return 0;
if ( MUTATION == 34 )
    return 1;
if ( MUTATION == 35 )
    return 1;
if ( MUTATION == 36 )
    return 2;
if ( MUTATION == 37 )
    return 3;
if ( MUTATION == 38 )
    return 4;
if ( MUTATION == 39 )
    return 0;
if ( MUTATION == 40 )
    return 1;
if ( MUTATION == 41 )
    return 2;
if ( MUTATION == 42 )
    return 3;
return -999;
}
int _FAQAS_selectOperation(){
if ( MUTATION == 0 )
    return 0;
if ( MUTATION == 1 )
    return 0;
if ( MUTATION == 2 )
    return 0;
if ( MUTATION == 3 )
    return 0;
if ( MUTATION == 4 )
    return 0;
if ( MUTATION == 5 )
    return 0;
if ( MUTATION == 6 )
    return 1;
if ( MUTATION == 7 )
    return 0;
if ( MUTATION == 8 )
    return 0;
if ( MUTATION == 9 )
    return 0;
if ( MUTATION == 10 )
    return 0;
if ( MUTATION == 11 )
    return 0;
if ( MUTATION == 12 )
    return 1;
if ( MUTATION == 13 )
    return 0;
if ( MUTATION == 14 )
    return 0;
if ( MUTATION == 15 )
    return 0;
if ( MUTATION == 16 )
    return 0;
if ( MUTATION == 17 )
    return 0;
if ( MUTATION == 18 )
    return 0;
if ( MUTATION == 19 )
    return 0;
if ( MUTATION == 20 )
    return 0;
if ( MUTATION == 21 )
    return 0;
if ( MUTATION == 22 )
    return 0;
if ( MUTATION == 23 )
    return 0;
if ( MUTATION == 24 )
    return 0;
if ( MUTATION == 25 )
    return 0;
if ( MUTATION == 26 )
    return 0;
if ( MUTATION == 27 )
    return 0;
if ( MUTATION == 28 )
    return 0;
if ( MUTATION == 29 )
    return 1;
if ( MUTATION == 30 )
    return 0;
if ( MUTATION == 31 )
    return 0;
if ( MUTATION == 32 )
    return 0;
if ( MUTATION == 33 )
    return 0;
if ( MUTATION == 34 )
    return 0;
if ( MUTATION == 35 )
    return 1;
if ( MUTATION == 36 )
    return 0;
if ( MUTATION == 37 )
    return 0;
if ( MUTATION == 38 )
    return 0;
if ( MUTATION == 39 )
    return 0;
if ( MUTATION == 40 )
    return 0;
if ( MUTATION == 41 )
    return 0;
if ( MUTATION == 42 )
    return 0;
return -999;
}


#define APPLY_ONE_MUTATION 0

int FAQAS_fmCov;
void _FAQAS_fmCoverage(int fm){
    switch (fm){
    case 0:
    FAQAS_fmCov++;
    break;
    case 1:
    FAQAS_fmCov++;
    break;
    default:
    break;
    }
}
//END _FAQAS_fmCoverage
//
// Copyright (c) University of Luxembourg 2020.
// Created by Fabrizio PASTORE, fabrizio.pastore@uni.lu, SnT, 2020.
// Modified by Oscar Eduardo CORNEJO OLIVARES, oscar.cornejo@uni.lu, SnT, 2020.
// Modified by Enrico VIGANO', enrico.vigano@uni.lu, SnT, 2021.
//

int _FAQAS_mutate(int *data, FaultModel *fm) {
  if (APPLY_ONE_MUTATION && _FAQAS_mutated == 1)
    return 0;

  if (MUTATION == -1)
    return 0;

  if (MUTATION == -2) {
    _FAQAS_fmCoverage(fm->ID);

#ifndef __cplusplus
    char *faqas_coverage_file = getenv("FAQAS_COVERAGE_FILE");
    FILE *coverage_file_pointer = fopen(faqas_coverage_file, "ab+");
#endif

    fprintf(coverage_file_pointer, "fm.ID: %d\n", fm->ID);

#ifndef __cplusplus
    fclose(coverage_file_pointer);
#endif

    return 0;
  }

  if (MUTATION == -3) {
    _FAQAS_fmCoverage(fm->ID);
    return 0;
  }

  if (MUTATION < fm->minOperation || MUTATION >= fm->maxOperation) {
    _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
    return 0;
  }

#ifdef _FAQAS_SINGLE_MUTATION

  if (global_mutation_counter > 0) {
    _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
    global_mutation_counter = global_mutation_counter + 1;
    return 0;
  }

#endif

#ifdef _FAQAS_MUTATION_PROBABILITY

  float random_check = ((float)rand() * (100)) / RAND_MAX;

  if (PROBABILITY < 0 || random_check > PROBABILITY) {
    _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
    global_mutation_counter = global_mutation_counter + 1;
    return 0;
  }

#endif

  int pos = _FAQAS_selectItem();
  int data_pos = _FAQAS_INITIAL_PADDING + pos;
  int op = _FAQAS_selectOperator();
  int opt = _FAQAS_selectOperation();

  int valueInt = 0;
  long int valueLong = 0;
  unsigned long long valueBin = 0;
  double valueDouble = 0;
  float valueFloat = 0;

  srand(time(NULL));
  //
  // Load the data
  //

  int span = fm->items[pos].span;

  int kk;
  int stepRead;
  unsigned long long row = 0;
  unsigned long long intermediate = 0;

  for (kk = 0; kk < (span); kk = kk + 1) {
    stepRead = 8 * sizeof(data[data_pos + kk]);
    intermediate = intermediate << stepRead;
    row = 0;
    memcpy(&row, &data[data_pos + kk], sizeof(data[data_pos + kk]));
    intermediate = (intermediate | row);
  }

  if (fm->items[pos].type == BIN) {
    unsigned long long fitToSize = (unsigned long long)intermediate;
    memcpy(&valueBin, &fitToSize, sizeof(valueBin));
  }

  if (fm->items[pos].type == INT) {
    unsigned int fitToSize = (unsigned int)intermediate;
    memcpy(&valueInt, &fitToSize, sizeof(valueInt));
  }

  if (fm->items[pos].type == DOUBLE) {
    unsigned long long int fitToSize = (unsigned long long int)intermediate;
    memcpy(&valueDouble, &fitToSize, sizeof(valueDouble));
  }

  if (fm->items[pos].type == FLOAT) {
    unsigned long int fitToSize = (unsigned long int)intermediate;
    memcpy(&valueFloat, &fitToSize, sizeof(valueFloat));
  }

  if (fm->items[pos].type == LONG) {
    unsigned long int fitToSize = (unsigned long int)intermediate;
    memcpy(&valueLong, &fitToSize, sizeof(valueLong));
  }

  //
  // Mutate the data
  //

  MutationOperator *OP = &(fm->items[pos].operators[op]);

  if (OP->type == HV) {

    if (sample == 1) {
      if (fm->items[pos].type == INT) {
        storedValueInt = valueInt;
      }

      if (fm->items[pos].type == DOUBLE) {
        storedValueDouble = valueDouble;
      }

      if (fm->items[pos].type == FLOAT) {
        storedValueFloat = valueFloat;
      }

      if (fm->items[pos].type == BIN) {
        storedValueBin = valueBin;
      }

      if (fm->items[pos].type == LONG) {
        storedValueBin = valueBin;
      }

      sample = 0;
      repeatCounter = OP->value;
    }

    if (repeatCounter > 0) {

      if (fm->items[pos].type == INT) {
        valueInt = storedValueInt;
      }

      if (fm->items[pos].type == DOUBLE) {
        valueDouble = storedValueDouble;
      }

      if (fm->items[pos].type == FLOAT) {
        valueFloat = storedValueFloat;
      }

      if (fm->items[pos].type == BIN) {
        valueBin = storedValueBin;
      }

      if (fm->items[pos].type == LONG) {
        valueLong = storedValueLong;
      }

      repeatCounter = repeatCounter - 1;
    }

    if (repeatCounter == 0) {
      sample = 1;
    }

    _FAQAS_mutated = 1;
    _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
  }

  if (OP->type == BF) {

    unsigned long long mask;
    // min = position of the first flippable bit from right to left
    int Min = OP->min;
    // max = position of the last flippable bit from right to left
    int Max = OP->max;
    // numberOfBits: (maximum) number of bits to change
    int numberOfBits = OP->value;
    // state: 1 mutate only bits ==1 and viceversa
    int State = OP->state;
    // random position of the bit to be changed
    int randomPosition;

    unsigned long long flipped;
    int avoidInfinite;
    int success = 1;

    if (State == 0) {
      int ii = 0;
      for (ii = 0; ii < numberOfBits; ii = ii + 1) {
        avoidInfinite = 0;
        flipped = valueBin;

        while (flipped == valueBin) {
          randomPosition = (rand() % (Max - Min + 1)) + Min;
          mask = FAQAS_pow_substitute(2, randomPosition);
          flipped = valueBin | mask;
          avoidInfinite = avoidInfinite + 1;
          if (avoidInfinite == numberOfBits * 10) {
            success = 0;
            break;
          }
        }
        valueBin = flipped;
      }
    } else if (State == 1) {
      int ii = 0;

      for (ii = 0; ii < numberOfBits; ii = ii + 1) {
        avoidInfinite = 0;
        flipped = valueBin;
        while (flipped == valueBin) {
          randomPosition = (rand() % (Max - Min + 1)) + Min;
          mask = FAQAS_pow_substitute(2, randomPosition);
          flipped = valueBin & ~mask;
          avoidInfinite = avoidInfinite + 1;
          if (avoidInfinite == numberOfBits * 10) {
            success = 0;
            break;
          }
        }
        valueBin = flipped;
      }
    } else {
      int ii = 0;

      for (ii = 0; ii < numberOfBits; ii = ii + 1) {
        flipped = valueBin;
        avoidInfinite = 0;

        while (flipped == valueBin) {
          randomPosition = (rand() % (Max - Min + 1)) + Min;
          mask = FAQAS_pow_substitute(2, randomPosition);
          flipped = valueBin & ~mask;
          if (flipped == valueBin) {
            flipped = valueBin | mask;
          }
          avoidInfinite = avoidInfinite + 1;
          if (avoidInfinite == numberOfBits * 10) {
            success = 0;
            break;
          }
        }
        valueBin = flipped;
      }
    }
    _FAQAS_operator_coverage(MUTATION, global_mutation_counter, success);
    _FAQAS_mutated = 1;
  }

  if (OP->type == VOR) {

    if (fm->items[pos].type == INT) {
      if (valueInt >= OP->min && valueInt <= OP->max) {
        if (opt == 0) {
          valueInt = OP->min - OP->delta;
          _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
        } else if (opt == 1) {
          valueInt = OP->max + OP->delta;
          _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
        } else {
          // FIXME: throw an error
        }
      } else {
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == LONG) {
      if (valueLong >= OP->min && valueLong <= OP->max) {
        if (opt == 0) {
          valueLong = OP->min - OP->delta;
          _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
        } else if (opt == 1) {
          valueLong = OP->max + OP->delta;
          _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
        } else {
          // FIXME: throw an error
        }
      } else {
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == DOUBLE) {
      if (valueDouble >= OP->min && valueDouble <= OP->max) {
        if (opt == 0) {
          valueDouble = OP->min - OP->delta;
          _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
        } else if (opt == 1) {
          valueDouble = OP->max + OP->delta;
          _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
        } else {
          // FIXME: throw an error
        }
      } else {
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == FLOAT) {
      if (valueFloat >= OP->min && valueFloat <= OP->max) {
        if (opt == 0) {
          valueFloat = OP->min - OP->delta;
          _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
        } else if (opt == 1) {
          valueFloat = OP->max + OP->delta;
          _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
        } else {
          // FIXME: throw an error
        }
      } else {
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }
  }

  if (OP->type == FVOR) {

    int fvor_success = 1;

    if (fm->items[pos].type == INT) {
      int upper = OP->max;
      int lower = OP->min;

      if (upper == lower) {
        valueInt = upper;
      } else if (upper < lower) {
        fvor_success = 0;
      }

      else {

        int randomNum = valueInt;
        int avoidInfinite = 0;

        while (valueInt == randomNum) {
          randomNum = (rand() % (upper - lower + 1)) + lower;
          avoidInfinite = avoidInfinite + 1;

          if (avoidInfinite == 1000) {
            fvor_success = 0;
            break;
          }
        }
        valueInt = randomNum;
      }
    }

    if (fm->items[pos].type == LONG) {
      long int upper = (long int)OP->max;
      long int lower = (long int)OP->min;

      if (upper == lower) {
        valueLong = upper;
        // FIXME: throw a warning
      } else if (upper < lower) {
        fvor_success = 0;
      } else {
        long int randomNum = valueLong;
        int avoidInfinite = 0;

        while (valueLong == randomNum) {
          randomNum = (rand() % (upper - lower + 1)) + lower;
          avoidInfinite = avoidInfinite + 1;

          if (avoidInfinite == 1000) {
            fvor_success = 0;
            break;
          }
        }
        valueLong = randomNum;
      }
    }

    if (fm->items[pos].type == DOUBLE) {

      double upper = OP->max;
      double lower = OP->min;

      if (upper == lower) {
        valueDouble = upper;
        // FIXME: throw a warning
      } else if (upper < lower) {
        // FIXME: throw an error
        fvor_success = 0;
      } else {
        double randomNum = valueDouble;
        int avoidInfinite = 0;

        while (valueDouble == randomNum) {
          randomNum = ((double)rand() * (upper - lower)) / RAND_MAX + lower;
          avoidInfinite = avoidInfinite + 1;

          if (avoidInfinite == 1000) {
            fvor_success = 0;
            break;
          }
        }
        valueDouble = randomNum;
      }
    }

    if (fm->items[pos].type == FLOAT) {
      float upper = OP->max;
      float lower = OP->min;

      if (upper == lower) {
        valueFloat = upper;
        // FIXME: throw a warning
      } else if (upper < lower) {
        // FIXME: throw an error
        fvor_success = 0;
      } else {
        float randomNum = valueFloat;
        int avoidInfinite = 0;

        while (valueFloat == randomNum) {
          randomNum = ((float)rand() * (upper - lower)) / RAND_MAX + lower;
          avoidInfinite = avoidInfinite + 1;

          if (avoidInfinite == 1000) {
            fvor_success = 0;
            break;
          }
        }
        valueFloat = randomNum;
      }
    }
    _FAQAS_operator_coverage(MUTATION, global_mutation_counter, fvor_success);
    _FAQAS_mutated = 1;

  } // end

  if (OP->type == VAT) {

    if (fm->items[pos].type == INT) {
      if (valueInt <= OP->threshold) {
        valueInt = OP->threshold + OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already above threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == LONG) {
      if (valueLong <= OP->threshold) {
        valueLong = OP->threshold + OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already above threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == DOUBLE) {
      if (valueDouble <= OP->threshold) {
        valueDouble = (double)OP->threshold + OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already above threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == FLOAT) {
      if (valueFloat <= (float)OP->threshold) {
        valueFloat = (float)OP->threshold + OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already above threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }
  }

  if (OP->type == FVAT) {

    if (fm->items[pos].type == INT) {
      if (valueInt > OP->threshold) {
        valueInt = OP->threshold - OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already above threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == LONG) {
      if (valueLong > OP->threshold) {
        valueLong = OP->threshold - OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already above threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == DOUBLE) {
      if (valueDouble > OP->threshold) {
        valueDouble = OP->threshold - OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already above threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == FLOAT) {
      if (valueFloat > OP->threshold) {
        valueFloat = OP->threshold - OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already above threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

  } // FINAL PAR

  if (OP->type == VBT) {

    if (fm->items[pos].type == INT) {
      if (valueInt >= OP->threshold) {
        valueInt = OP->threshold - OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already below threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == LONG) {
      if (valueLong >= OP->threshold) {
        valueLong = OP->threshold - OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == DOUBLE) {
      if (valueDouble >= (double)OP->threshold) {
        valueDouble = (double)OP->threshold - (double)OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already below threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == FLOAT) {
      if (valueFloat >= (float)OP->threshold) {
        valueFloat = (float)OP->threshold - OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already below threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }
  }

  if (OP->type == FVBT) {

    if (fm->items[pos].type == INT) {
      if (valueInt < OP->threshold) {
        valueInt = OP->threshold + OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already above threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == LONG) {
      if (valueLong < OP->threshold) {
        valueLong = OP->threshold + OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already above threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == DOUBLE) {
      if (valueDouble < OP->threshold) {
        valueDouble = OP->threshold + OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already above threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == FLOAT) {
      if (valueFloat < OP->threshold) {
        valueFloat = OP->threshold + OP->delta;
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
      } else {
        // value already above threshold
        _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 0);
      }
      _FAQAS_mutated = 1;
    }

  } // FINAL PAR

  if (OP->type == IV) {
    int IVsuccess = 0;

    if (fm->items[pos].type == INT) {
      if (valueInt != OP->value) {
        valueInt = OP->value;
        IVsuccess = 1;
      }
    }

    if (fm->items[pos].type == LONG) {
      if (valueLong != OP->value) {
        valueLong = (long)OP->value;
        IVsuccess = 1;
      }
    }

    if (fm->items[pos].type == DOUBLE) {
      if (valueDouble != OP->value) {
        valueDouble = (double)OP->value;
        IVsuccess = 1;
      }
    }

    if (fm->items[pos].type == FLOAT) {
      if (valueFloat != OP->value) {
        valueFloat = (float)OP->value;
        IVsuccess = 1;
      }
    }

    _FAQAS_operator_coverage(MUTATION, global_mutation_counter, IVsuccess);
    _FAQAS_mutated = 1;
  }

  if (OP->type == SS) {

    if (fm->items[pos].type == INT) {
      int shift = (int)OP->delta;
      valueInt = (int)valueInt + shift;
    }

    if (fm->items[pos].type == LONG) {
      long int shift = (long int)OP->delta;
      valueLong = (long int)valueLong + shift;
    }

    if (fm->items[pos].type == DOUBLE) {
      double shift = (double)OP->delta;
      valueDouble = (double)valueDouble + shift;
    }

    if (fm->items[pos].type == FLOAT) {
      float shift = (float)OP->delta;
      valueFloat = (float)valueFloat + shift;
    }

    _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
    _FAQAS_mutated = 1;
  }

  if (OP->type == INV) {

    int inv_success = 1;

    if (fm->items[pos].type == INT) {
      int upper = OP->max;
      int lower = OP->min;

      if (upper == lower) {
        valueInt = upper;
      } else if (upper < lower) {
        inv_success = 0;
      }

      else {

        int randomNum = valueInt;
        int avoidInfinite = 0;

        while (valueInt == randomNum) {
          randomNum = (rand() % (upper - lower + 1)) + lower;
          avoidInfinite = avoidInfinite + 1;

          if (avoidInfinite == 1000) {
            inv_success = 0;
            break;
          }
        }
        valueInt = randomNum;
      }
    }

    if (fm->items[pos].type == LONG) {
      long int upper = (long int)OP->max;
      long int lower = (long int)OP->min;

      if (upper == lower) {
        valueLong = upper;
        // FIXME: throw a warning
      } else if (upper < lower) {
        inv_success = 0;
      } else {
        long int randomNum = valueLong;
        int avoidInfinite = 0;

        while (valueLong == randomNum) {
          randomNum = (rand() % (upper - lower + 1)) + lower;
          avoidInfinite = avoidInfinite + 1;

          if (avoidInfinite == 1000) {
            inv_success = 0;
            break;
          }
        }
        valueLong = randomNum;
      }
    }

    if (fm->items[pos].type == DOUBLE) {

      double upper = OP->max;
      double lower = OP->min;

      if (upper == lower) {
        valueDouble = upper;
        // FIXME: throw a warning
      } else if (upper < lower) {
        // FIXME: throw an error
        inv_success = 0;
      } else {
        double randomNum = valueDouble;
        int avoidInfinite = 0;

        while (valueDouble == randomNum) {
          randomNum = ((double)rand() * (upper - lower)) / RAND_MAX + lower;
          avoidInfinite = avoidInfinite + 1;

          if (avoidInfinite == 1000) {
            inv_success = 0;
            break;
          }
        }
        valueDouble = randomNum;
      }
    }

    if (fm->items[pos].type == FLOAT) {
      float upper = OP->max;
      float lower = OP->min;

      if (upper == lower) {
        valueFloat = upper;
        // FIXME: throw a warning
      } else if (upper < lower) {
        // FIXME: throw an error
        inv_success = 0;
      } else {
        float randomNum = valueFloat;
        int avoidInfinite = 0;

        while (valueFloat == randomNum) {
          randomNum = ((float)rand() * (upper - lower)) / RAND_MAX + lower;
          avoidInfinite = avoidInfinite + 1;

          if (avoidInfinite == 1000) {
            inv_success = 0;
            break;
          }
        }
        valueFloat = randomNum;
      }
    }
    _FAQAS_operator_coverage(MUTATION, global_mutation_counter, inv_success);
    _FAQAS_mutated = 1;
  } // end

  if (OP->type == ASA) {

    if (fm->items[pos].type == INT) {
      int Tr = OP->threshold;
      int De = OP->delta;
      int Va = OP->value;

      if (valueInt >= Tr) {
        valueInt = Tr + ((valueInt - Tr) * Va) + De;
      }

      if (valueInt < Tr) {
        valueInt = Tr - ((valueInt - Tr) * Va) + De;
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == DOUBLE) {
      double Tr = OP->threshold;
      double De = OP->delta;
      double Va = OP->value;

      if (valueDouble >= Tr) {
        valueDouble = Tr + ((valueDouble - Tr) * Va) + De;
      }

      if (valueDouble < Tr) {
        valueDouble = Tr - ((valueDouble - Tr) * Va) + De;
      }
      _FAQAS_mutated = 1;
    }

    if (fm->items[pos].type == FLOAT) {

      float Tr = OP->threshold;
      float De = OP->delta;
      float Va = OP->value;

      if (valueFloat >= Tr) {
        valueFloat = Tr + ((valueFloat - Tr) * Va) + De;
      }

      if (valueFloat < Tr) {
        valueFloat = Tr - ((valueFloat - Tr) * Va) + De;
      }
      _FAQAS_mutated = 1;
    }
    _FAQAS_operator_coverage(MUTATION, global_mutation_counter, 1);
  }

  if (_FAQAS_mutated != 1) {
    return 0;
  }

  // Store the data

  unsigned long long fullNumber = 0;

  switch (fm->items[pos].type) {

  case BIN:
    memcpy(&fullNumber, &valueBin, sizeof(valueBin));
    break;

  case INT:
    memcpy(&fullNumber, &valueInt, sizeof(valueInt));
    break;

  case DOUBLE:
    memcpy(&fullNumber, &valueDouble, sizeof(valueDouble));
    break;

  case FLOAT:
    memcpy(&fullNumber, &valueFloat, sizeof(valueFloat));
    break;

  case LONG:
    memcpy(&fullNumber, &valueLong, sizeof(valueLong));
    break;
  }

  int counter = 0;
  int stepWrite = 0;

  while (counter < span) {
    stepWrite = 8 * sizeof(data[data_pos + counter]);
    int startSlice = (span - counter - 1) * stepWrite;
    int endSlice = (span - counter) * stepWrite - 1;
    unsigned long long slice =
        _FAQAS_slice_it_up(fullNumber, startSlice, endSlice);
    memcpy(&data[data_pos + counter], &slice, sizeof(data[data_pos + counter]));
    counter = counter + 1;
  }
  global_mutation_counter = global_mutation_counter + 1;
  return _FAQAS_mutated;
}
