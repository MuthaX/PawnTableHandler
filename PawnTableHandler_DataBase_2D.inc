#if(!defined mysql_included)
	#error Include file of the MySQL plugin have to be included before PawnTableHandler_Database.inc!
#endif
#if(!defined __TABLEHANDLER_INCLUDED__)
	#error The file PawnTableHandler_Main.inc have to be included before PawnTableHandler_Database.inc!
#endif
//==============================================================================
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
		count_items = -1,
		index_0, index_1, index_temp
	;
	#if !defined TABLEHANDLER_STRUCTTEST_DISABLE
	if( error_counter ) {
		printf("[TableHandler_DB_parseDirect_2D] cant save table due to the problems in table structure (found %d errors)!", error_counter);
		return count_items;
	}
	#endif
	#if(defined cache_get_field_type)
	cache_get_row_count(rows_number);
	#else
	rows_number = cache_get_row_count(db_handle);
	#endif
	for(row_index = 0; row_index < rows_number; ++row_index) {
		index_0 = index_1 = -1;
		for(column_index = 0; column_index < table_info_size; ++column_index) {
			data_type =		table_info[column_index][E_TABLEDATA_TYPE];
			data_offset =	table_info[column_index][E_TABLEDATA_OFFSET];
			if( data_type == '@' ) {
				#if defined cache_get_field_type
				cache_get_value_index_int(row_index, column_index, index_temp);
				#else
				index_temp = cache_get_row_int(row_index, column_index, db_handle);
				#endif
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
			} else {
				if( data_type == 'd' || data_type == 'h' ) {
					#if defined cache_get_field_type	// R40
					cache_get_value_index_int(row_index, column_index, dest_array[index_0][index_1][data_offset]);
					#else	// <=R39
					dest_array[index_0][index_1][data_offset] = cache_get_row_int(row_index, column_index, db_handle);
					#endif
				} else if( data_type == 'f' ) {
					#if defined cache_get_field_type	// R40
					cache_get_value_index_float(row_index, column_index, Float:dest_array[index_0][index_1][data_offset]);
					#else	// <=R39
					dest_array[index_0][index_1][data_offset] = _:cache_get_row_float(row_index, column_index, db_handle);
					#endif
				} else if( data_type == 's' ) {
					data_size =		table_info[column_index][E_TABLEDATA_SIZE];
					#if defined cache_get_field_type	// R40
					cache_get_value_index(row_index, column_index, dest_array[index_0][index_1][data_offset], data_size);
					#else	// <=R39
					cache_get_row(row_index, column_index, dest_array[index_0][index_1][data_offset], db_handle, data_size);
					#endif
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
		count_items = -1,
		index_0, index_1, index_temp
	;
	#if !defined TABLEHANDLER_STRUCTTEST_DISABLE
	if( error_counter ) {
		printf("[TableHandler_DB_parseContent_2D] cant save table due to the problems in table structure (found %d errors)!", error_counter);
		return count_items;
	}
	#endif
	#if(defined cache_get_field_type)
	cache_get_row_count(rows_number);
	#else
	rows_number = cache_get_row_count(db_handle);
	#endif
	for(row_index = 0; row_index < rows_number; ++row_index) {
		index_0 = index_1 = -1;
		for(column_index = 0; column_index < table_info_size; ++column_index) {
			data_type =		table_info[column_index][E_TABLEDATA_TYPE];
			data_offset =	table_info[column_index][E_TABLEDATA_OFFSET];
			if( data_type == '@' ) {
				#if defined cache_get_field_type
				cache_get_value_name_int(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], index_temp);
				#else
				index_temp = cache_get_field_content_int(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], db_handle);
				#endif
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
			} else {
				if( data_type == 'd' || data_type == 'h' ) {
					#if defined cache_get_field_type	// R40
					cache_get_value_name_int(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], dest_array[index_0][index_1][data_offset]);
					#else	// <=R39
					dest_array[index_0][index_1][data_offset] = cache_get_field_content_int(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], db_handle);
					#endif
				} else if( data_type == 'f' ) {
					#if defined cache_get_field_type	// R40
					cache_get_value_name_float(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], Float:dest_array[index_0][index_1][data_offset]);
					#else	// <=R39
					dest_array[index_0][index_1][data_offset] = _:cache_get_field_content_float(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], db_handle);
					#endif
				} else if( data_type == 's' ) {
					data_size =		table_info[column_index][E_TABLEDATA_SIZE];
					#if defined cache_get_field_type	// R40
					cache_get_value_name(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], dest_array[index_0][index_1][data_offset], data_size);
					#else	// <=R39
					cache_get_field_content(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], dest_array[index_0][index_1][data_offset], db_handle, data_size);
					#endif
				}
			}
		}
		count_items++;
	}
	return count_items;
}
