#if(!defined mysql_included)
	#error Include file of the MySQL plugin have to be included before PawnTableHandler_Database.inc!
#endif
#if(!defined __TABLEHANDLER_INCLUDED__)
	#error The file PawnTableHandler_Main.inc have to be included before PawnTableHandler_Database.inc!
#endif
//==============================================================================
enum e_tablehandle_db_struct_mode {
	E_TABLEDATA_RES_AUTODETECT,
	E_TABLEDATA_RES_DIFFERENT,
	E_TABLEDATA_RES_ISSAME
};

//	Returns true if the defined table structure and currently active query result is same.
stock TableHandler_DB_IsSameStructRes(
	#if !defined cache_get_field_type	// <=R39
	const db_handle,
	#endif
	const table_info[][e_table_model_struct_info],
	const table_info_size
) {
	new
		field_count,
		field_index,
		field_name[TABLEHANDLER_MAX_DBFIELD_NAME]
	;
	#if defined cache_get_field_type	// R40
		cache_get_field_count(field_count);
	#else	// <=R39
		field_count = cache_get_field_count(db_handle);
	#endif
	if( field_count != table_info_size ) {
		return false;
	}
	for(field_index = 0; field_index < field_count; ++field_index) {
		#if defined cache_get_field_type	// R40
			cache_get_field_name(field_index, field_name, sizeof(field_name));
		#else
			cache_get_field_name(field_index, field_name, db_handle, sizeof(field_name));
		#endif
		if( strcmp(table_info[field_index][E_TABLEDATA_DB_FIELD_NAME], field_name, true) ) {
			return false;
		}
	}
	return true;
}

//	Parsing query results with direct use of table structure.
stock TableHandler_DB_parseDirect(
	#if !defined cache_get_field_type	// <=R39
	const db_handle,
	#endif
	dest_array[][],
	const dest_size,
	const table_info[][e_table_model_struct_info],
	const table_info_size,
	const table_name[]
) {
	new
		#if(!defined TABLEHANDLER_STRUCTTEST_DISABLE)
		error_counter = TableHandler_isInvalidStruct(table_info, table_info_size, table_name),
		#endif
		is_indexed = (table_info[0][E_TABLEDATA_TYPE] == '@'),
		row_index,
		rows_number,
		column_index,
		array_row = -1,
		data_type,
		data_offset,
		data_size,
		count_items = -1
	;
	#if !defined TABLEHANDLER_STRUCTTEST_DISABLE
	if( error_counter ) {
		printf("[TableHandler_DB_parseDirect(%s)] cant save table due to the problems in table structure (found %d errors)!", table_name, error_counter);
		return count_items;
	}
	#endif
	#if(defined cache_get_field_type)
	cache_get_row_count(rows_number);
	#else
	rows_number = cache_get_row_count(db_handle);
	#endif
	for(row_index = 0; row_index < rows_number; ++row_index) {
		column_index = is_indexed;
		if( is_indexed ) {
			#if defined cache_get_field_type
			cache_get_value_index_int(row_index, 0, array_row);
			#else
			array_row = cache_get_row_int(row_index, 0, db_handle);
			#endif
		} else {
			array_row++;
		}
		if( array_row < dest_size && array_row >= 0 ) {
			while( column_index < table_info_size ) {
				data_type =		table_info[column_index][E_TABLEDATA_TYPE];
				data_offset =	table_info[column_index][E_TABLEDATA_OFFSET];
				if( data_type == 'd' || data_type == 'h' ) {
					#if defined cache_get_field_type	// R40
					cache_get_value_index_int(row_index, column_index, dest_array[array_row][data_offset]);
					#else	// <=R39
					dest_array[array_row][data_offset] = cache_get_row_int(row_index, column_index, db_handle);
					#endif
				} else if( data_type == 'f' ) {
					#if defined cache_get_field_type	// R40
					cache_get_value_index_float(row_index, column_index, Float:dest_array[array_row][data_offset]);
					#else	// <=R39
					dest_array[array_row][data_offset] = _:cache_get_row_float(row_index, column_index, db_handle);
					#endif
				} else if( data_type == 's' ) {
					data_size =		table_info[column_index][E_TABLEDATA_SIZE];
					#if defined cache_get_field_type	// R40
					cache_get_value_index(row_index, column_index, dest_array[array_row][data_offset], data_size);
					#else	// <=R39
					cache_get_row(row_index, column_index, dest_array[array_row][data_offset], db_handle, data_size);
					#endif
				}
				column_index++;
			}
			count_items++;
		} else {
			printf("[TableHandler_DB_parseDirect(%s)] invalid index found(%d) @ line=%d!", table_name, array_row, row_index);
			if( !is_indexed ) {
				break;
			}
		}
	}
	return count_items;
}

//	Parsing SQL query results with use field names as indexes.
stock TableHandler_DB_parseContent(
	#if !defined cache_get_field_type	// <=R39
	const db_handle,
	#endif
	dest_array[][],
	const dest_size,
	const table_info[][e_table_model_struct_info],
	const table_info_size,
	const table_name[]
) {
	new
		#if(!defined TABLEHANDLER_STRUCTTEST_DISABLE)
		error_counter = TableHandler_isInvalidStruct(table_info, table_info_size, table_name),
		#endif
		is_indexed = (table_info[0][E_TABLEDATA_TYPE] == '@'),
		rows_number,
		row_index,
		column_index,
		array_row = -1,
		data_type,
		data_offset,
		data_size,
		count_items = -1
	;
	#if(defined cache_get_field_type)
	cache_get_row_count(rows_number);
	#else
	rows_number = cache_get_row_count(db_handle);
	#endif
	#if(!defined TABLEHANDLER_STRUCTTEST_DISABLE)
	if( error_counter ) {
		printf("[TableHandler_DB_parseContent(%s)] cant load table from query result due to the problems in table structure (found %d errors)!", table_name, error_counter);
		return count_items;
	}
	#endif
	for(row_index = 0; row_index < rows_number; ++row_index) {
		column_index = is_indexed;
		if( is_indexed ) {
			#if(defined cache_get_field_type)
			cache_get_value_name_int(row_index, table_info[0][E_TABLEDATA_DB_FIELD_NAME], array_row);
			#else
			array_row = cache_get_field_content_int(row_index, table_info[0][E_TABLEDATA_DB_FIELD_NAME], db_handle);
			#endif
		} else {
			array_row++;
		}
		if( array_row < dest_size && array_row >= 0 ) {
			while( column_index < table_info_size ) {
				data_type =		table_info[column_index][E_TABLEDATA_TYPE];
				data_offset =	table_info[column_index][E_TABLEDATA_OFFSET];
				if( data_type == 'd' || data_type == 'h' ) {
					#if(defined cache_get_field_type)
					cache_get_value_name_int(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], dest_array[array_row][data_offset]);
					#else
					dest_array[array_row][data_offset] = cache_get_field_content_int(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], db_handle);
					#endif
				} else if( data_type == 'f' ) {
					#if(defined cache_get_field_type)
					cache_get_value_name_float(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], Float:dest_array[array_row][data_offset]);
					#else
					dest_array[array_row][data_offset] = _:cache_get_field_content_float(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], db_handle);
					#endif
				} else if( data_type == 's' ) {
					data_size =		table_info[column_index][E_TABLEDATA_SIZE];
					#if(defined cache_get_field_type)
					cache_get_value_name(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], dest_array[array_row][data_offset], data_size);
					#else
					cache_get_field_content(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], dest_array[array_row][data_offset], db_handle, data_size);
					#endif
				}
				column_index++;
			}
			count_items++;
		} else {
			printf("[TableHandler_DB_parseContent (%s)] invalid index found(%d) @ line=%d!", table_name, array_row, row_index);
			if( !is_indexed ) {
				break;
			}
		}
	}
	return count_items;
}

//	Parsing query results with selection parsing strategy (and saving auto-determined result).
stock TableHandler_DB_parseStrategy(
	#if !defined cache_get_field_type	// <=R39
	const db_handle,
	#endif
	&e_tablehandle_db_struct_mode:parsing_stategy,
	dest_array[][],
	const dest_size,
	const table_info[][e_table_model_struct_info],
	const table_info_size,
	const table_name[]
) {
	if( parsing_stategy == E_TABLEDATA_RES_AUTODETECT ) {
		new compare_result;
		#if(defined cache_get_field_type)
		compare_result = TableHandler_DB_IsSameStructRes(table_info, table_info_size);
		#else
		compare_result = TableHandler_DB_IsSameStructRes(db_handle, table_info, table_info_size);
		#endif
		if( compare_result ) {
			parsing_stategy = E_TABLEDATA_RES_ISSAME;
		} else {
			parsing_stategy = E_TABLEDATA_RES_DIFFERENT;
		}
	}
	new result;
	if( parsing_stategy == E_TABLEDATA_RES_ISSAME ) {
		#if defined cache_get_field_type	// <=R39
		result = TableHandler_DB_parseDirect(table_name, dest_array, dest_size, table_info, table_info_size);
		#else
		result = TableHandler_DB_parseDirect(table_name, db_handle, dest_array, dest_size, table_info, table_info_size);
		#endif
	} else {
		#if defined cache_get_field_type	// <=R39
		result = TableHandler_DB_parseContent(table_name, dest_array, dest_size, table_info, table_info_size);
		#else
		result = TableHandler_DB_parseContent(table_name, db_handle, dest_array, dest_size, table_info, table_info_size);
		#endif
	}
	return result;
}
