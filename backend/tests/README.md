# Tests

## Balancer Test

With `TestBalancer` you can test whether a (new) balancer provides a correct subregion division. New testcases can be added by by `push_back`-ing an instance of `struct TestCase` to the vector `testCases`.

```cpp
struct TestCase {
	/**
	* The name of your testcase
	*/
	std::string testName;

	/**
	* Number of workers to balance for
	*/
	int nodeCount;
	
	/**
	* The the balancing strategy to use
	*/
	Balancer* balancer;
	
	/**
	* The region to balance
	*/
	Region* region;
};
```
The test needs to be recompiled after every change. To compile the first time you will need to create the `build` directory:

```bash
$ mkdir build
```
Then compile using `cmake`:

```bash
$ cd build
$ cmake ..
$ make
```

You can then run the tests from the executable:

```bash
$ ./testbalancer
```

Printing of all the subregions can be enabled by using the verbose option (`-v`).