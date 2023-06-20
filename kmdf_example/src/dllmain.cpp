#include <ntifs.h>
#include <../include/ke_importer.hpp>

long dll_main()
{
	ke_fn(DbgPrint)("hello world \n");

	return STATUS_SUCCESS;
}