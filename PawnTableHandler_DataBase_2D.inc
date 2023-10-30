#if(!defined mysql_included)
	#error Include file of the MySQL plugin have to be included before PawnTableHandler_Database.inc!
#endif
#if(!defined __TABLEHANDLER_INCLUDED__)
	#error The file PawnTableHandler_Main.inc have to be included before PawnTableHandler_Database.inc!
#endif
#if(!defined __TABLEHANDLER_DB_DEFS__)
	#error The file PawnTableHandler_DB_Definitions.inc have to be included before PawnTableHandler_Database.inc!
#endif
//==============================================================================
static string_buffer[4096];

//	Parsing query results with direct use of table structure.
stock TableHandler_DB_parseDirect_2D(
	#if !defined cache_get_field_type	// <=R39
	const db_handle,
	#endif
	dest_array[][][],
	const size_0,
	const size_1,
	const table_info[][e_table_model_struct_info],
	const table_info_size,
	const table_name[]
) {
	#pragma unused table_name
	new
		#if(!defined TABLEHANDLER_STRUCTTEST_DISABLE)
		error_counter = TableHandler_isInvalidStruct(table_info, table_info_size, table_name),
		#endif
		row_index,
		rows_number,
		column_index,
		data_type,
		data_offset,
		data_size,
		e_tablehanlder_option:data_options,
		count_items = -1,
		index_0, index_1, index_temp
	;
	#if !defined TABLEHANDLER_STRUCTTEST_DISABLE
	if( error_counter ) {
		printf("[TableHandler_DB_parseDirect_2D] cant save table due to the problems in table structure (found %d errors)!", error_counter);
		return count_items;
	}
	#endif
	PTH_MYSQL_GET_ROWCOUNT(rows_number, db_handle);
	for(row_index = 0; row_index < rows_number; ++row_index) {
		index_0 = index_1 = -1;
		for(column_index = 0; column_index < table_info_size; ++column_index) {
			data_type =		table_info[column_index][E_TABLEDATA_TYPE];
			data_offset =	table_info[column_index][E_TABLEDATA_OFFSET];
			data_options =	e_tablehanlder_option:table_info[column_index][E_TABLEDATA_OPTIONS];
			if( data_type == '@' ) {
				PTH_MYSQL_FIELDNAME_GETINT(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], index_temp, db_handle);
				if( data_offset == 0 ) {
					index_0 = index_temp;
					if( index_0 >= size_0 ) {
						break;
					}
				} else if( data_offset == 1 ) {
					index_1 = index_temp;
					if( index_1 >= size_1 ) {
						break;
					}
				}
			} else if( data_type == 'd' || data_type == 'h' ) {
				PTH_MYSQL_FIELDID_GETINT(row_index, column_index, dest_array[index_0][index_1][data_offset], db_handle);
			} else if( data_type == 'f' ) {
				PTH_MYSQL_FIELDID_GETFLOAT(row_index, column_index, dest_array[index_0][index_1][data_offset], db_handle);
			} else if( data_type == 's' ) {
				data_size =		table_info[column_index][E_TABLEDATA_SIZE];
				if( data_options & E_TABLEHANDLER_OPTION_PACK ) {
					PTH_MYSQL_FIELDID_GETSTR(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], string_buffer, db_handle, sizeof(string_buffer));
					strpack(dest_array[index_0][index_1][data_offset], string_buffer, data_size);
				} else {
					PTH_MYSQL_FIELDID_GETSTR(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], dest_array[index_0][index_1][data_offset], db_handle, data_size);
				}
			}
		}
		count_items++;
	}
	return count_items;
}

//	Parsing SQL query results with use field names as indexes.
stock TableHandler_DB_parseContent_2D(
	#if !defined cache_get_field_type	// <=R39
	const db_handle,
	#endif
	dest_array[][][],
	const size_0,
	const size_1,
	const table_info[][e_table_model_struct_info],
	const table_info_size,
	const table_name[]
) {
	#pragma unused table_name
	new
		#if(!defined TABLEHANDLER_STRUCTTEST_DISABLE)
		error_counter = TableHandler_isInvalidStruct(table_info, table_info_size, table_name),
		#endif
		row_index,
		rows_number,
		column_index,
		data_type,
		data_offset,
		data_size,
		e_tablehanlder_option:data_options,
		count_items = -1,
		index_0, index_1, index_temp
	;
	#if !defined TABLEHANDLER_STRUCTTEST_DISABLE
	if( error_counter ) {
		printf("[TableHandler_DB_parseContent_2D] cant save table due to the problems in table structure (found %d errors)!", error_counter);
		return count_items;
	}
	#endif
	PTH_MYSQL_GET_ROWCOUNT(rows_number, db_handle);
	for(row_index = 0; row_index < rows_number; ++row_index) {
		index_0 = index_1 = -1;
		for(column_index = 0; column_index < table_info_size; ++column_index) {
			data_type =		table_info[column_index][E_TABLEDATA_TYPE];
			data_offset =	table_info[column_index][E_TABLEDATA_OFFSET];
			data_options =	e_tablehanlder_option:table_info[column_index][E_TABLEDATA_OPTIONS];
			if( data_type == '@' ) {
				PTH_MYSQL_FIELDNAME_GETINT(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], index_temp, db_handle);
				if( data_offset == 0 ) {
					index_0 = index_temp;
					if( index_0 >= size_0 ) {
						break;
					}
				} else if( data_offset == 1 ) {
					index_1 = index_temp;
					if( index_1 >= size_1 ) {
						break;
					}
				}
			} else if( data_type == 'd' || data_type == 'h' ) {
				PTH_MYSQL_FIELDNAME_GETINT(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], dest_array[index_0][index_1][data_offset], db_handle);
			} else if( data_type == 'f' ) {
				PTH_MYSQL_FIELDNAME_GETFLOAT(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], dest_array[index_0][index_1][data_offset], db_handle);
			} else if( data_type == 's' ) {
				data_size =		table_info[column_index][E_TABLEDATA_SIZE];
				if( data_options & E_TABLEHANDLER_OPTION_PACK ) {
					PTH_MYSQL_FIELDNAME_GETSTR(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], string_buffer, db_handle, sizeof(string_buffer));
					strpack(dest_array[index_0][index_1][data_offset], string_buffer, data_size);
				} else {
					PTH_MYSQL_FIELDNAME_GETSTR(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], dest_array[index_0][index_1][data_offset], db_handle, data_size);
				}
			}
		}
	}
	return count_items;
}
