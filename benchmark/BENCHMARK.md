## Benchmarking

TODO: Document

**Initial Benchmarking Results**

```
System's no of threads: 4
## Scheduling & Synchronization Overhead ##
simple_pool_noop duration: 32224677 microsecs (32224.7 ms)
advanced_pool_noop duration: 15447055 microsecs (15447.1 ms)
gcd_dispatch_noop duration: 685656 microsecs (685.656 ms)
## Small CPU Bound Task ##
simple_pool_cpu_bound duration: 24983726 microsecs (24983.7 ms)
advanced_pool_cpu_bound duration: 14117910 microsecs (14117.9 ms)
gcd_dispatch_cpu_bound duration: 1239796 microsecs (1239.8 ms)
## Memory Bound Task ##
simple_pool_memory_bound duration: 30873566 microsecs (30873.6 ms)
advanced_pool_memory_bound duration: 28259462 microsecs (28259.5 ms)
gcd_dispatch_memory_bound duration: 28414654 microsecs (28414.7 ms)
## END BENCHMARK ##
```