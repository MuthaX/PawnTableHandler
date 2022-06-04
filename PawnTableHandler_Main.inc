#define __TABLEHANDLER_INCLUDED__

#if !defined TABLEHANDLER_MAX_STRING_SIZE
	#define TABLEHANDLER_MAX_STRING_SIZE	(2048)
#endif
#if !defined TABLEHANDLER_MAX_COLUMNS
	#define TABLEHANDLER_MAX_COLUMNS		(64)
#endif
#if !defined TABLEHANDLER_MAX_DBFIELD_NAME
	#define TABLEHANDLER_MAX_DBFIELD_NAME	(32)
#endif

#if TABLEHANDLER_MAX_STRING_SIZE < 1
	#error TABLEHANDLER_MAX_STRING_SIZE must be greater than zero!
#endif
#if TABLEHANDLER_MAX_COLUMNS < 1
	#error TABLEHANDLER_MAX_COLUMNS must be greater than zero!
#endif
#if TABLEHANDLER_MAX_DBFIELD_NAME < 1
	#error TABLEHANDLER_MAX_DBFIELD_NAME must be greater than zero!
#endif
/*
Example of definition table structure.
static const TestTableStruct[][e_table_model_struct_info] = {
	{'@',	0,	0,	12,	"id"},
	{'d',	1,	0,	12,	"value"},
	{'f',	2,	4,	12,	"speed"},
	{'s',	3,	0,	16,	"name"}
};
*/
enum e_table_model_struct_info {
	E_TABLEDATA_TYPE,				// defines row id, decimal integer, float, hex, string, ...;
	E_TABLEDATA_OFFSET,				// defines offset in array;
	E_TABLEDATA_PRECISION,			// defines number of digits after dot at writing float;
	E_TABLEDATA_SIZE,				// defines width for integers and cells for strings;
	E_TABLEDATA_BINSYMBOLS,			// defines used space in bytes[if BINUTF=false] or UTF-8 symbols (in binary files) for strings;
	E_TABLEDATA_OPTIONS,			// defines boolean parameters of column.
	E_TABLEDATA_DB_FIELD_NAME[TABLEHANDLER_MAX_DBFIELD_NAME]	// defines name to access db field(column);
};
enum e_tablehanlder_option {
	E_TABLEHANDLER_OPTION_UTF = 	1,
	E_TABLEHANDLER_OPTION_PACK =	2
};
static const valid_data_types[] = {	// These values have to be sorted.
	'@',	// Integer, that points at row in destination array, have to be only in first column;
	'd',	// Integer (decimal);
	'f',	// Float;
	'h',	// Integer (hex);
	's'		// String (have to be not null!);
};
//	Returns true if test_data_type is invalid data type for table handler.
stock TableHandler_isValidDataType(const test_data_type) {
	if( test_data_type < valid_data_types[0] || test_data_type > valid_data_types[sizeof(valid_data_types) - 1] ) {
		return false;
	}
	for(new data_type = 0; data_type < sizeof(valid_data_types); ++data_type) {
		if( test_data_type == valid_data_types[data_type] ) {
			return true;
		}
	}
	return false;
}
//	Returns true if delimiter character is invalid.
stock TableHandler_isInvalidDelimiter(const delimiter) {
	if( !delimiter ) {
		return true;
	} else if( (delimiter > '0' - 1) && (delimiter < '9' + 1) ) {
		return true;
	}
	return false;
}
//	Returns number of problems with structure of table. Better use for debugging!
stock TableHandler_isInvalidStruct(const table_info[][e_table_model_struct_info], const table_info_size) {
	new
		table_column,
		alter_column,
		error_counter = 0,
		data_type,
		data_offset,
		data_precision,
		data_size,
		data_bytes,
		data_options,
		alter_offset,
		data_to_a, data_cells_a,
		data_to_b, data_cells_b,
		column_name[TABLEHANDLER_MAX_DBFIELD_NAME]
	;
	if( table_info_size < 1 ) {
		printf("[TableStruct_isInvalid] invalid number of columns in table structure(%d < 1)!", table_info_size);
		error_counter++;
	}
	for(table_column = 0; table_column < table_info_size; ++table_column) {
		data_type =			table_info[table_column][E_TABLEDATA_TYPE];
		data_offset =		table_info[table_column][E_TABLEDATA_OFFSET];
		data_precision =	table_info[table_column][E_TABLEDATA_PRECISION];
		data_size =			table_info[table_column][E_TABLEDATA_SIZE];
		data_bytes =		table_info[table_column][E_TABLEDATA_BINSYMBOLS];
		data_options =		table_info[table_column][E_TABLEDATA_OPTIONS];
		strmid(column_name, table_info[table_column][E_TABLEDATA_DB_FIELD_NAME], 0, TABLEHANDLER_MAX_DBFIELD_NAME, sizeof(column_name));
		if( !TableHandler_isValidDataType(data_type) ) {
			printf("[TableStruct_isInvalid] invalid data type(%c) for column %d(%s)!", data_type, table_column, column_name);
			error_counter++;
		}
		if( data_type == '@' && table_column > 0 ) {
			printf("[TableStruct_isInvalid] @column have to be first, found at column=%d(%s)!", table_column, column_name);
			error_counter++;
		}
		if( data_offset < 0 ) {
			printf("[TableStruct_isInvalid] invalid offset(%d) for column %d(%s)!", data_offset, table_column, column_name);
			error_counter++;
		}
		if( data_precision < 0 ) {
			printf("[TableStruct_isInvalid] invalid precision(%d) for column %d(%s)!", data_precision, table_column, column_name);
			error_counter++;
		}
		if( data_size < 0 ) {
			printf("[TableStruct_isInvalid] invalid precision(%d) for column %d(%s)!", data_size, table_column, column_name);
			error_counter++;
		}
		if( !column_name[0] ) {
			printf("[TableStruct_isInvalid] column %d must be non zero name!", table_column);
			error_counter++;
		} else if( table_info[table_column][E_TABLEDATA_DB_FIELD_NAME][TABLEHANDLER_MAX_DBFIELD_NAME - 1] ) {
			printf("[TableStruct_isInvalid] DB field name must be null terminated in column %d!", table_column);
			error_counter++;
		}
		if( data_type == 's' ) {
			if( data_bytes < 2 ) {
				printf("[TableStruct_isInvalid] column %d(%s) have to have at least 2 bytes size(%s)!", table_column, column_name, data_bytes);
				error_counter++;
			}
			if( e_tablehanlder_option:data_options & E_TABLEHANDLER_OPTION_PACK ) {
				if( 4 * data_bytes != data_size ) {
					printf("[TableStruct_isInvalid] column %d(%s) incorrect bytes-size(%d != %d)!", data_bytes, data_size / 4);
					error_counter++;
				}
			} else {
				if( data_bytes != data_size ) {
					printf("[TableStruct_isInvalid] column %d(%s) incorrect bytes-size(%d != %d)!", data_bytes, data_size);
					error_counter++;
				}
			}
		} else {
			if( data_bytes < 1 || data_bytes > 4 ) {
				printf("[TableStruct_isInvalid] column %d(%s) have invalid bytes-size for valiable(%d)!", table_column, column_name, data_bytes);
				error_counter++;
			}
		}
		data_cells_a = 1;
		data_to_a = data_offset;
		if( data_type == 's' ) {
			data_cells_a = data_size;
			data_to_a += data_cells_a - 1;
		}
		for(alter_column = table_column + 1; alter_column < table_info_size; ++alter_column) {
			data_to_b = alter_offset = table_info[alter_column][E_TABLEDATA_OFFSET];
			data_cells_b = 1;
			if( table_info[alter_column][E_TABLEDATA_TYPE] == 's' ) {
				data_cells_b = table_info[alter_column][E_TABLEDATA_SIZE];
				data_to_b += data_cells_b - 1;
			}
			if( !strcmp(column_name, table_info[alter_column][E_TABLEDATA_DB_FIELD_NAME], true) ) {
				printf("[TableStruct_isInvalid] found repeated field name in different columns(%d & %d)!", table_column, alter_column);
				error_counter++;
			}
			new
				pos_min = ((alter_offset < data_offset) ? alter_offset : data_offset),
				pos_max = ((data_to_b > data_to_a) ? data_to_b : data_to_a)
			;
			if( (pos_max - pos_min + 1) < (data_cells_a + data_cells_b) ) {
				printf("[TableStruct_isInvalid] columns %d & %d are intersecting!", table_column, alter_column);
				error_counter++;
			}
		}
	}
	return error_counter;
}
