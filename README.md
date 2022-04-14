*Copyright (c) University of Luxembourg 2022.*

*Created by Enrico VIGANO, enrico.vigano@uni.lu, SnT, 2022.*

# DAMAt example

## Introduction

This is an example of how to apply the _DAMAt_ tool and procedure to improve an existing test suite.
This procedure has been performed on Ubuntu 20.04.4 LTS.

We decided to use _libCSP_, a C library implementing the Cubesat Space Protocol (CSP), a small protocol stack written to facilitate communication between the components of a CubeSat.
Additional information on _libCSP_ can be found at this link https://github.com/libcsp/libcsp.

To apply the _DAMAt_ procedure we need a Software Under Test and the relative test suite to evaluate.
_LibCSP_ does not come with a test suite, but it includes some examples; starting from one of these examples, we devised an intentionally flawed integration test suite.
The test suite contains a single test case that emulates a server/client configuration over loop-back and verifies that a minimum number of packets is transmitted by the client and correctly received by the server in a given time.

## Installation procedure

Once uncompressed, inside the _libcsp\_workspace_ directory you will find three sub-folders:
* _damat-pipeline_: this folder contains all the scripts and utilities necessary to apply _DAMAt_.
* _libcsp_: this folder contains the source code for libCSP.
* _test\_suite_: this folder contains the test suite under test that we are going to try and improve.

To compile _libCSP_ and its test cases you will need to install the following packages:

```shell
sudo apt-get python gcc pkg-config libsocketcan-dev libzmq3-dev
```
This version of _libCSP_ has been slightly modified with the insertion of DAMAt's mutation probes (see Step 3: Inserting the mutation probes).
To compile it,  you will need to go to the _libcsp_ folder and run the following command.

```shell
./waf distclean configure build --mutation-opt -1 --singleton TRUE --with-os=posix --enable-rdp --enable-promisc --enable-hmac --enable-dedup --enable-can-socketcan --with-driver-usart=linux --enable-if-zmqhub --enable-examples
```
The flag _--mutation-opt -1_ will set the mutation probes to an inactive state.

To execute the original test case go to the _test\_suite_ folder and run the following commands:

```shell
make clean
make test_01

```
The test should pass.

## Executing DAMAt

### Step 1: Fault Model Specification

The first step for applying the _DAMAt_ procedure is specifying a fault model.
The fault model must be written in the _csv_ format supported by DAMAt. You can find it here:

```
damat-pipeline/fault_model_libcsp.csv
```

This _csv_ file contains two _Fault Models_ implementing a total of 39 _Mutation Operators_, which will generate the mutants.
Then you can make sure that the _DAMAt\_configure.sh_ scripts contain the correct path to the fault model file.

Another _csv_ file, _test.csv_ should contain the following:

```
test_01,10000
```
The first column should contain the names of the test cases and the second the normal maximum execution time.

### Step 2: Generating the mutation API

To generate the mutation API, run the following commands:

``` shell
cd damat-pipeline
./DAMAt_probe_generation.sh
```

You should see that a new file is generated in the _damat-pipeline_ folder: _FAQAS\_dataDrivenMutator.h_. This file will contain the implementation of the mutation probes that will be used to modify the SUT's buffer data and generate the mutants.
Another newly generated file, _function\_calls.out_ contains the definitions of the functions implementing the mutation probes. There is a probe for every different fault model.

### Step 3: Insert the mutation probes

We will insert the mutation probes in a specific _libCSP_ source file:
```
libcsp/src/csp_io.c
```
This file contains the definitions of the two functions that we will target with the probe insertion strategy:
* _csp\_send_
* _csp\_read_

these functions take as input the data structure _csp\_conn\_t_, which we are going to mutate.
By looking at the content of the file
```
libcsp/csp_io_mutated.c
```
you can see how the probes were embedded in the code. This is an example:

```c

csp_packet_t * csp_read(csp_conn_t * conn, uint32_t timeout) {

	/* mutation probe */

	int damat_buffer_read[7];

	damat_buffer_read[0] = 0;
	damat_buffer_read[1] = conn->idin.pri;
	damat_buffer_read[2] = conn->idin.src;
	damat_buffer_read[3] = conn->idin.dst;
	damat_buffer_read[4] = conn->idin.dport;
	damat_buffer_read[5] = conn->idin.sport;
	damat_buffer_read[6] = conn->idin.flags;

	mutate_FM_Read(damat_buffer_read);

	conn->idin.pri = damat_buffer_read[1];
	conn->idin.src = damat_buffer_read[2];
	conn->idin.dst = damat_buffer_read[3];
	conn->idin.dport = damat_buffer_read[4];
	conn->idin.sport = damat_buffer_read[5];
	conn->idin.flags = damat_buffer_read[6];

	/* end of the probe */

	csp_packet_t * packet = NULL;

	if ((conn == NULL) || (conn->state != CONN_OPEN)) {
		return NULL;
	}

#if (CSP_USE_RDP)
	// RDP: timeout can either be 0 (for no hang poll/check) or minimum the "connection timeout"
	if (timeout && (conn->idin.flags & CSP_FRDP) && (timeout < conn->rdp.conn_timeout)) {
		timeout = conn->rdp.conn_timeout;
	}
#endif

	if (csp_queue_dequeue(conn->rx_queue, &packet, timeout) != CSP_QUEUE_OK) {
		return NULL;
	}

#if (CSP_USE_RDP)
	/* Packet read could trigger ACK transmission */
	if ((conn->idin.flags & CSP_FRDP) && conn->rdp.delayed_acks) {
		csp_rdp_check_ack(conn);
	}
#endif

	return packet;
}

```
The extra lines of code are because, in its current form, _DAMAt_ can only mutate arrays.
The file also contains the following inclusion:
```c
#include "FAQAS_dataDrivenMutator.h"
```
To insert the probes, you just need to substitute _csp\_io\_mutated.c_ to the original file and copy the _FAQAS\_dataDrivenMutator.h_ in the same folder.

### Step 4: Compile mutants

To enable this step, which will be automatically performed by the _DAMAt_ pipeline, the user will need to modify this script:
```
damat-pipeline/DAMAt_run_tests.sh
```
to include the commands to compile the SUT.
By taking a look at it, you will see how it has been done for this particular case.

```shell
# here the user must invoke the compilation of the SUT

compilation_folder="/home/vagrant/libcsp_workspace/libcsp"

pushd $compilation_folder

echo "$deco"
echo "compiling test"
echo "$deco"

./waf distclean configure build --mutation-opt $mutant_id $EXTRA_FLAGS_SINGL --with-os=posix --enable-rdp --enable-promisc --enable-hmac --enable-dedup --enable-can-socketcan --with-driver-usart=linux --enable-if-zmqhub

```
### Step 5: Execute the test suite

As with Step 4, Step 5 will be performed automatically by the pipeline, but the user must take a few preparatory steps.
First, the user must provide the _csv_ file containing the name of the test case and a normal execution time (useful to set a timeout in case a mutant should cause an infinite loop);
You can find the one used in this example here:
```
damat-pipeline/tests.csv
```

Then the commands to run the test cases must be included in the proper space of the script
```
damat-pipeline/DAMAt_run_tests.sh
```

In this case, this is the code that has been added:

```shell
# here the user shall call the execution of the current test case,

  pushd /home/vagrant/libcsp_workspace/test_suite

  echo "$deco"
  echo "$(tput setaf 1) RUNNING THE TEST NOW! $(tput sgr0)"
  echo "$deco"

  # timeout 30 ./build/examples/csp_server_client -t
  make clean
  make $tst

  EXEC_RET_CODE=$?

  echo "$deco"
  echo "$(tput setaf 2) FINITO! $(tput sgr0)"
  echo "$deco"

  popd

###############################################################################
```
The _$tst_ variable is read from the first column of the csv file. When new test cases are added they must be included in that file to be executed during the procedure.

### Step 6: Generate the results

At this point to execute the test suite against the mutants and gather the results you just need to run the pipeline by typing:

```shell
cd damat-pipeline
./DAMAt_mutants_launcher.sh

```
When the execution is complete you will see this message:


TODO: add the final output


## Improving on the existing test suite

After the first execution of _DAMAt_, the metrics describing the performance of the test suite are the following:
* _Fault Model Coverage_ 100%
* _Mutation Operation Coverage_ 98%
* _Mutation Score_ 52%

At a first glance we can see that not all the input partitions have been covered, since the _MOC_ is < 100%, and some oracles are missing or incomplete since the _MS_ is < 100%.

The file
```
results/final_mutants_table.csv
```
contains information on the status of every single mutant. This file will allow us to see what fault models, data items, and input partitions, in particular, are not well tested due to the test suite's shortcomings.
For example the _DataItem_ column contains information on which data item in the buffer is targeted by the mutant. In particular:

* 1 = conn-><idin/idout>.pri
* 2 = conn-><idin/idout>.src
* 3 = conn-><idin/idout>.dst
* 4 = conn-><idin/idout>.dport
* 5 = conn-><idin/idout>.sport
* 6 = conn-><idin/idout>.flags

We will add test cases based on the data gathered by DAMAt to improve the test suite.
A summary of this process, including metrics and tables is contained in the file
```
summary_of_the_results.xlsx
```

### Improving the Mutation Operation Coverage

We can improve the _Mutation Operation Coverage_ by adding new test cases that exercise partitions not covered by the test suite such as the ones targeted by mutants that were _NOT\_APPLIED_.

By looking at the mutants that were _NOT\_APPLIED_ we can identify input partitions not covered by the test suite:
*  there is no test case that covers a value of _conn->idin.pri_ and _conn->idout.pri_ > 3;

### Improving the Mutation Score

We can improve the _Mutation Score_ by adding new test cases that contain oracles on the values modified by _LIVE_ mutants. In this case, this being an integration test suite, our primary focus is to check whether the different components (server and client) interact correctly and if the connection data contained in the structure _csp\_conn\_t_ is correctly handled and preserved through these interactions.

By looking at the mutants that were _APPLIED_ but not _KILLED_ by the test suite, we notice that they belong to some specific members of the _csp\_conn\_t_ structure:
*  _conn->idin.pri_ and _conn->idout.pri_, which define the priority of the connection;
* _conn->idin.src_, _conn->idout.src_, _conn->idin.dst_, and _conn->idout.src_, which represent the source and destination;
* _conn->idin.sport_, which represents the source port.

#### Test 02: Priority (pri)

A test case containing an oracle that checks if the _conn->idin.pri_ and _conn->idout.pri_ coincide between server and client should detect eventual mismanagement of the priority in the connection interfaces, and kill the mutants emulating these kinds of faults, that were previously _APPLIED_ but not _KILLED_.
In the test the client will send 5 packages with the four different priorities defined by _libCSP_:
* CSP_PRIO_CRITICAL (0)
* CSP_PRIO_HIGH (1)
* CSP_PRIO_NORM (2)
* CSP_PRIO_LOW (3)

Then it will test what happens if the priority is not defined by _libCSP_, for example, if it is equal to 6, thus hopefully improving the _Mutation Operation Coverage_, leading to a more extensive test suite.
The content of _conn->idin.pri_ as received by the server will be checked against the priority established by the client when connecting.
The test case is implemented in the file
```
test_suite/test_02/test_02.c
```
and can be executed by compiling _libCSP_, moving to the _test\_suite_ folder, and executing the following command:
```shell
make test_02
```

Then you can add the test to the _damat-pipeline/tests.csv_ so that it will be recognized and executed by _DAMAt_:
The file should now contain these lines:

```
test_01,10000
test_02,10000
```

After adding _test\_02_ to the test suite and re-executing _DAMAt_, the metrics should become the following:
* _Fault Model Coverage_ 100%
* _Mutation Operation Coverage_ 100%
* _Mutation Score_ 61%

#### Test 03: Source (src) and Destination (dst)

A test case containing an oracle that checks the content of _conn->idin.dst_, _conn->idin.src_, _conn->idout.dst_, _conn->idout.src_   should detect eventual alteration of these values between server and client, and kill the mutants emulating these kinds of faults, that were previously _APPLIED_ but not _KILLED_.

The test consists of the client sending 5 messages to the server. For every message, the content of the aforementioned members of the _csp_conn_t_ data structure shall be checked for discrepancies between server and client.


The test case is implemented in the file
```
test_suite/test_03/test_03.c
```
and can be executed by compiling _libCSP_, moving to the _test\_suite_ folder, and executing the following command:
```shell
make test_03
```

Then you can add the test to the _damat-pipeline/tests.csv_ so that it will be recognized and executed by _DAMAt_:
The file should now contain these lines:

```
test_01,10000
test_02,10000
test_03,10000
```

After adding _test\_03_ to the test suite and re-executing _DAMAt_, the metrics will become the following:
* _Fault Model Coverage_ 100%
* _Mutation Operation Coverage_ 100%
* _Mutation Score_ 67%

#### Test 04: Source Port (sport) and Destination Port (dport)

A test case containing an oracle that checks the content of _conn->idin.sport_ and _conn->idin.dport_ should detect eventual alteration of these values between server and client, and kill the mutants emulating these kinds of faults, that were previously _APPLIED_ but not _KILLED_.

The test consists of the client sending 4 messages to the server. For every message, the content of the aforementioned members of the _csp_conn_t_ data structure shall be checked for discrepancies between server and client. In particular, the server-side _dport_ shall coincide with the client-side _sport_ and vice-versa.

The test case is implemented in the file
```
test_suite/test_04/test_04.c
```
and can be executed by compiling _libCSP_, moving to the _test\_suite_ folder, and executing the following command:
```shell
make test_04
```

Then you can add the test to the _damat-pipeline/tests.csv_ so that it will be recognized and executed by _DAMAt_:
The file should now contain these lines:
```
test_01,10000
test_02,10000
test_03,10000
test_04,10000
```

After adding _test\_04_ to the test suite and re-executing _DAMAt_, the metrics will become the following:
* _Fault Model Coverage_ 100%
* _Mutation Operation Coverage_ 100%
* _Mutation Score_ 81%

#### Test 05: Flags (flags)

A test case containing an oracle that checks the content of _conn->idin.flags_ and _conn->idin.flags_ should detect eventual alteration of these values between server and client, and kill the mutants emulating these kinds of faults, that were previously _APPLIED_ but not _KILLED_.

The test case is implemented in the file
```
test_suite/test_04/test_04.c
```
and can be executed by compiling _libCSP_, moving to the _test\_suite_ folder, and executing the following command:
```shell
make test_04
```

Then you can add the test to the _damat-pipeline/tests.csv_ so that it will be recognized and executed by _DAMAt_:
The file should now contain these lines:

```
test_01,10000
test_02,10000
test_03,10000
test_04,10000
```

After adding _test\_05_ to the test suite and re-executing _DAMAt_, the metrics will become the following:
* _Fault Model Coverage_ 100%
* _Mutation Operation Coverage_ 100%
* _Mutation Score_ 95%

## Conclusion
This example is intended to show that, with the help of DAMAt, a user can obtain valuable indication on how to improve a test suite.
The new test suite for  _libCSP_, while intentionally still very limited and simple, should be more capable of identifying problems in the SUT than it was at the start.
