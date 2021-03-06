# MultiCL: Mutli-platform OpenCL with automatic device scheduling
MultiCL extends the OpenCL standard by providing scope for automatic scheduling at the context and command queue levels. In this way, the command queue remains as an abstraction of a set of OpenCL commands and is not bound to any physical device for the duration of the program. We have also added a demo scheduler to show how the OpenCL extensions are sufficient to provide intelligent decisions on command placement. For the implementation, we have extended the SnuCL platform, which already provides the extremely useful capability of cross-platform support of OpenCL vendors and device types. 

# Bibtex entry for citation
```
@InProceedings{aji-queue-sch-cluster15, 
	author =	{Aji, Ashwin M. and Pena, Antonio J. and Balaji, Pavan and Feng, Wu-chun},
	title = 	"{Automatic Command Queue Scheduling for Task-Parallel Workloads in OpenCL}",
	booktitle =	{IEEE Cluster},
	address =	{Chicago, Illinois},
	month =	{September},
	year =	{2015},
}
```
