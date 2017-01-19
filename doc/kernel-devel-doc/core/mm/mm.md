#MM Module
##Summary
This module supports heap and policy of paging.
##Sub Modules
* <a href="./paging.md">Paging</a>

##Interfaces
###`void core_mm_init();`
####Description
Initialize this module.
####Parameters
None.
####Return value
None.

###`void core_mm_core_init(u32 cpuid);`
####Description
Initialize datastructures of a new cpu core.
####Parameters
* cpuid
ID of new cpu core.

####Return value
None

###`void core_mm_core_release(u32 cpuid);`
####Description
Releas the datastructures of a cpu core.
####Parameters
* cpuid
ID of new cpu core.

####Return value
None

