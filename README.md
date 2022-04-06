
*Copyright (c) University of Luxembourg 2020.*

*Created by Enrico VIGANO, enrico.vigano@uni.lu, SnT, 2022.*

# DAMAt example

## Introduction

This is an example on how to apply the _DAMAt_ tool and procedure to improve an existing test suite.
We decided to use _libCSP_, a C library implementing the Cubesat Space Protocol (CSP), a small protocol stack written to facilitate communication between the components of a CubeSat.
Additional information on libCSP can be found at this link https://github.com/libcsp/libcsp.

To apply the _DAMAt_ procedure we need a Software Under Test and the relative test suite to evaluate.
_LibCSP_ does not come with a test suite, but it includes some examples; starting from one of this examples, we devised an intentionally flawed integration test suite.
The test suite emulates a server/client configuration over loop-back and verifies that a minimum number of packets is transmitted by the client and correctly received by the server in a given time.

Inside the _libcsp\_workspace_ directory you will find three sub-folders:
* _damat-pipeline_: this folder contains all the scripts and utilities necessary to apply _DAMAt_.
* _libcsp_: this folder contains the source code for libCSP.
* _test\_suite_: this folder contains the test suite under test that we are going to try and improve.

TODO: add a script to easily compile libCSP

TODO: add a section on how to install libcsp and damat dependencies

TODO: maybe virtual machine?

## Executing DAMAt

### Step 1: Fault Model Specification

The first step for applying the _DAMAt_ procedure is specifying a fault model. The motivation between the _libCSP_ fault model are included in the _libcsp_fault_model.docx_ file.
The fault model must be converted to the _csv_ format supported by DAMAt. You can find it here:

```
damat-pipeline/fault_model_libcsp.csv
```

This _csv_ file contains two _Fault Models_ implementing a total of 55 _Mutation Operators_, which will generate the mutants.
Then you can make sure that the _DAMAt\_configure.sh_ scripts contains the correct path to the fault model file.

### Step 2: Generating the mutation API

To generate the mutation API, run the following commands:

``` shell
cd damat-pipeline
./DAMAt_probe_generation.sh
```

You should see a new file being generated in the _damat-pipeline_ folder: _FAQAS\_dataDrivenMutator.h_. This file will contain the implementation of the mutation probes that will be used to modify the SUT's buffer data and generate the mutants.
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
The extra line of code are due to the fact that, in its current form, _DAMAt_ can only mutate arrays.
The file also contains the following inclusion:
```c
#include "FAQAS_dataDrivenMutator.h"
```
To insert the probes, you just need to substitute _csp\_io\_mutated.c_ to the original file and copy the _FAQAS\_dataDrivenMutator.h_ in the same folder.

### Step 4: Compile mutants

To enable this step, that will be automatically performed by the _DAMAt_ pipeline, the user will need to modify this script:
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

As with Step 4, Step 5 will be performed autimatically by the pipeline, but the user must take a few preparatory steps.
First, the user must provide another simple _csv_ file containing the name of the test case and a normal execution time (useful to set a timeout in case a mutant should cause an infinite loop);
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
* _Mutation Operation Coverage_ 75.9%
* _Mutation Score_ 50%

At a first glance we can see that not all the input partitions have been covered, since the _MOC_ is < 100%, and some oracles are missing or incomplete, since the _MS_ is < 100%.
This will already point us in the right direction when we think of what we need to do to improve the test suite.

The file
```
results/final_mutants_table.csv
```
contains information on the status of every single mutant. This file will allow us to see what fault models, data items and input partitions in particular are not well tested due to the test suite's shortcomings.
A summary of the metrics and table is contained in a more readable format in the file
```
summary_of_the_results.xlsx
```
### Improving the Mutation Score

We can improve the mutation score by adding new test cases that exercise the _Data Items_ not covered by the test suite and contain oracles on their values. In this case, this being an integration test suite, our primary focus is to check whether the different components (server and client) interact in a correct manner and if the connection data contained in the structure _csp\_conn\_t_ is correctly handled and preserved through these interactions.

#### Test 02: Priority (pri)

After adding _test\_02_ to the test suite and re-executing _DAMAt_, the metrics will become the following:
* _Fault Model Coverage_ 100%
* _Mutation Operation Coverage_ 75.9%
* _Mutation Score_ 50%

#### Test 03: Source (src) and Destination (dst)

After adding _test\_03_ to the test suite and re-executing _DAMAt_, the metrics will become the following:
* _Fault Model Coverage_ 100%
* _Mutation Operation Coverage_ 75.9%
* _Mutation Score_ 50%

#### Test 04: Source Port (sport) and Destination Port (dport)

After adding _test\_04_ to the test suite and re-executing _DAMAt_, the metrics will become the following:
* _Fault Model Coverage_ 100%
* _Mutation Operation Coverage_ 75.9%
* _Mutation Score_ 50%

#### Test 05: Flags (flags)

After adding _test\_05_ to the test suite and re-executing _DAMAt_, the metrics will become the following:
* _Fault Model Coverage_ 100%
* _Mutation Operation Coverage_ 75.9%
* _Mutation Score_ 50%
