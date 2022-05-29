# PawnTableHandler
Procedures for loading and saving tables from/into different sources using only structure description.

## Defines
**\_\_TABLEHANDLER_INCLUDED\_\_** - used for determinate use of PawnTableHandler.  
**TABLEHANDLER_MAX_STRING_SIZE** - maximum size of buffered string when reading file through fread. Default size 2048 cells. Can be defined with custom value.  
**TABLEHANDLER_MAX_COLUMNS** - maximum number of columns in procedures that are using fread. Default number of columns is 64. Can be defined with custom value.  
**TABLEHANDLER_MAX_DBFIELD_NAME** - maximum size of db-field name in table structure. Default size is 32 cells. Can be defined with custom value.  
**TABLEHANDLER_STRUCTTEST_DISABLE** - define it by yourself to disable checking table structure before handling table. May be useful at production state (undefine while debugging).  

## Table structure model
**E_TABLEDATA_TYPE** - (character) defines row id, decimal integer, float, hex, string, ...;  
**E_TABLEDATA_OFFSET** - defines offset in array;  
**E_TABLEDATA_PRECISION** - defines number of digits after dot at writing float;  
**E_TABLEDATA_SIZE** - defines width for integers and cells for strings;  
**E_TABLEDATA_DB_FIELD_NAME** - defines name to access db field(column);  

## Allowed data types
Put as character where it used.  
> * @ - Integer, that points at row in destination array, **have to be only in first column**;
> * d - Integer (decimal);
> * f - Float;
> * h - Integer (hex);
> * s - String (have to be not null!);

## General procedures
__TableHandler_isValidDataType(const test_data_type)__
> Returns true if test_data_type is invalid data type for table handler.

__TableHandler_isInvalidDelimiter(const delimiter)__
> Returns true if delimiter character is invalid.

TableHandler_isInvalidStruct(const table_info[][e_table_model_struct_info], const table_info_size)
> Returns number of problems with structure of table. Better use for debugging!

## File-related procedures
__TableHandler_loadStructFile(filepath[], delimiter, dest_array[][], dest_size, table_info[][e_table_model_struct_info], table_info_size);__
> Reads structured table from file into dest_array.  
> Returns negative value if any serious problem found, otherwise number of loaded rows.  
> * filepath[] - Destination of file to load.
> * delimiter - Character used for split columns.
> * dest_array\[][] - Destination array.
> * dest_size - Number of rows in destination array.
> * table_info\[][e_table_model_struct_info] - Structure of table.
> * table_info_size - Number of columns in table structure.

__TableHandler_loadLargeStruct(filepath[], delimiter, dest_array[][], dest_size, table_info\[][e_table_model_struct_info], table_info_size, bool:use_utf);__
> Reads structured table from file into dest_array.  
> There is no limitation in string size or number of columns at cost of low performance.  
> Returns negative value if any serious problem found, otherwise number of loaded rows.  
> * filepath[] - Destination of file to load.
> * delimiter - Character used for split columns.
> * dest_array\[][] - Destination array.
> * dest_size - Number of rows in destination array.
> * table_info\[][e_table_model_struct_info] - Structure of table.
> * table_info_size - Number of columns in table structure.
> * bool:use_utf

__TableHandler_saveStructFile(filepath[], delimiter, src_array[][], src_size,table_info[][e_table_model_struct_info], table_info_size);__
> Saves structured table into file from dest_array.  
> Returns negative value if any serious problem found, otherwise number of written rows.  
> * filepath[] - Destination of file to save.
> * delimiter - Character used for split columns.
> * src_array\[][] - Source array.
> * src_size - Number of rows in source array.
> * table_info\[][e_table_model_struct_info] - Structure of table.
> * table_info_size						// Number of columns in table structure.

### Example
```Pawn
#include <PawnTableHandler\PawnTableHandler_Main.inc>
#include <PawnTableHandler\PawnTableHandler_Files.inc>
#include <PawnTableHandler\PawnTableHandler_DataBase.inc>

enum e_array_struct {
	E_MODELID,
	E_INTEGER,
	Float:E_FLOAT,
	E_STRING[16]
};
static const TestTableStruct[][e_table_model_struct_info] = {
	{'@',	0,	0,	1,	"id"},
	{'d',	E_INTEGER,	0,	1,	"value"},
	{'f',	E_FLOAT,	4,	1,	"speed"},
	{'s',	E_STRING,	0,	16,	"name"}
};
new Array[2][e_array_struct];

main() {
	TableHandler_loadStructFile("test/table_handler/source_table.txt", '\t', Array, sizeof(Array), TestTableStruct, sizeof(TestTableStruct));
	TableHandler_loadLargeStruct("test/table_handler/source_table.txt", '\t', Array, sizeof(Array), TestTableStruct, sizeof(TestTableStruct), false);
	for(new array_index = 0; array_index < sizeof(Array); ++array_index) {
		printf(
			"Array content@%d: (%d)(%d)(%f)(%s)",
			array_index,
			Array[array_index][E_MODELID],
			Array[array_index][E_INTEGER],
			Array[array_index][E_FLOAT],
			Array[array_index][E_STRING]
		);
	}
	TableHandler_saveStructFile("test/table_handler/dest_table.txt", '\t', Array, sizeof(Array), TestTableStruct, sizeof(TestTableStruct));
}
```
##  DataBase-related procedures
enum e_tablehandle_db_struct_mode {
	E_TABLEDATA_RES_AUTODETECT,
	E_TABLEDATA_RES_DIFFERENT,
	E_TABLEDATA_RES_ISSAME
};

__TableHandler_DB_IsSameStructRes(db_handle, table_info[][e_table_model_struct_info], table_info_size)__
> Returns true if the defined table structure and currently active query result is same.  
> * db_handle - (used only with MySQL-plugin up to R39)
> * table_info\[][e_table_model_struct_info] - Structure of table.
> * table_info_size - Number of columns in table structure.

__TableHandler_DB_parseDirect(db_handle, dest_array[][], dest_size, table_info[][e_table_model_struct_info], table_info_size)__
> Parsing query results with direct use of table structure.  
> Returns negative value if any serious problem found, otherwise number of loaded rows.  
> * db_handle - (used only with MySQL-plugin up to R39)
> * dest_array\[][] - Destination array.
> * dest_size - Number of rows in destination array.
> * table_info\[][e_table_model_struct_info] - Structure of table.
> * table_info_size - Number of columns in table structure.

__TableHandler_DB_parseContent(db_handle, dest_array[][], dest_size, table_info[][e_table_model_struct_info], table_info_size)__
> Parsing SQL query results with use field names as indexes.  
> Returns negative value if any serious problem found, otherwise number of loaded rows.  
> * db_handle - (used only with MySQL-plugin up to R39)
> * dest_array\[][] - Destination array.
> * dest_size - Number of rows in destination array.
> * table_info\[][e_table_model_struct_info] - Structure of table.
> * table_info_size - Number of columns in table structure.

__TableHandler_DB_parseStrategy(db_handle, &e_tablehandle_db_struct_mode:parsing_stategy, dest_array[][], dest_size, table_info[][e_table_model_struct_info], table_info_size)__
> Parsing query results with selection parsing strategy (and saving auto-determined result).  
> Returns negative value if any serious problem found, otherwise number of loaded rows.  
> * db_handle - (used only with MySQL-plugin up to R39)
> * parsing_stategy - defines parsing strategy and saves it when changed
> * dest_array\[][] - Destination array.
> * dest_size - Number of rows in destination array.
> * table_info\[][e_table_model_struct_info] - Structure of table.
> * table_info_size - Number of columns in table structure.

