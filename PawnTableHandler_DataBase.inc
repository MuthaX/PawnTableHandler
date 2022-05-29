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
	const db_handle,
	const table_info[][e_table_model_struct_info],
	const table_info_size
) {
	new
		field_count,
		field_index,
		field_name[TABLEHANDLER_MAX_DBFIELD_NAME]
	;
	field_count = cache_get_field_count(db_handle);
	if( field_count != table_info_size ) {
		return false;
	}
	for(field_index = 0; field_index < field_count; ++field_index) {
		cache_get_field_name(field_index, field_name, db_handle, sizeof(field_name));
		if( strcmp(table_info[field_index][E_TABLEDATA_DB_FIELD_NAME], field_name, true) ) {
			return false;
		}
	}
	return true;
}

//	Parsing query results with direct use of table structure.
stock TableHandler_DB_parseDirect(
	const db_handle,
	dest_array[][],
	const dest_size,
	const table_info[][e_table_model_struct_info],
	const table_info_size
) {
	new
		#if(!defined TABLEHANDLER_STRUCTTEST_DISABLE)
		error_counter = TableHandler_isInvalidStruct(table_info, table_info_size),
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
		printf("[TableHandler_DB_parseDirect] cant save table due to the problems in table structure (found %d errors)!", error_counter);
		return count_items;
	}
	#endif
	rows_number = cache_get_row_count(db_handle);
	for(row_index = 0; row_index < rows_number; ++row_index) {
		column_index = is_indexed;
		if( is_indexed ) {
			array_row = cache_get_row_int(row_index, 0, db_handle);
		} else {
			array_row++;
		}
		if( array_row < dest_size && array_row >= 0 ) {
			while( column_index < table_info_size ) {
				data_type =		table_info[column_index][E_TABLEDATA_TYPE];
				data_offset =	table_info[column_index][E_TABLEDATA_OFFSET];
				if( data_type == 'd' || data_type == 'h' ) {
					dest_array[array_row][data_offset] = cache_get_row_int(row_index, column_index, db_handle);
				} else if( data_type == 'f' ) {
					dest_array[array_row][data_offset] = _:cache_get_row_float(row_index, column_index, db_handle);
				} else if( data_type == 's' ) {
					data_size =		table_info[column_index][E_TABLEDATA_SIZE];
					cache_get_row(row_index, column_index, dest_array[array_row][data_offset], db_handle, data_size);
				}
				column_index++;
			}
			count_items++;
		} else {
			printf("[TableHandler_DB_parseDirect] invalid index found(%d) @ line=%d!", array_row, row_index);
			if( !is_indexed ) {
				break;
			}
		}
	}
	return count_items;
}

//	Parsing SQL query results with use field names as indexes.
stock TableHandler_DB_parseContent(
	const db_handle,
	dest_array[][],
	const dest_size,
	const table_info[][e_table_model_struct_info],
	const table_info_size
) {
	new
		#if(!defined TABLEHANDLER_STRUCTTEST_DISABLE)
		error_counter = TableHandler_isInvalidStruct(table_info, table_info_size),
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
	rows_number = cache_get_row_count(db_handle);
	#if(!defined TABLEHANDLER_STRUCTTEST_DISABLE)
	if( error_counter ) {
		printf("[TableHandler_DB_parseContent] cant load table from[%d] due to the problems in table structure (found %d errors)!", db_handle, error_counter);
		return count_items;
	}
	#endif
	for(row_index = 0; row_index < rows_number; ++row_index) {
		column_index = is_indexed;
		if( is_indexed ) {
			array_row = cache_get_field_content_int(row_index, table_info[0][E_TABLEDATA_DB_FIELD_NAME], db_handle);
		} else {
			array_row++;
		}
		if( array_row < dest_size && array_row >= 0 ) {
			while( column_index < table_info_size ) {
				data_type =		table_info[column_index][E_TABLEDATA_TYPE];
				data_offset =	table_info[column_index][E_TABLEDATA_OFFSET];
				if( data_type == 'd' || data_type == 'h' ) {
					dest_array[array_row][data_offset] = cache_get_field_content_int(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], db_handle);
				} else if( data_type == 'f' ) {
					dest_array[array_row][data_offset] = _:cache_get_field_content_float(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], db_handle);
				} else if( data_type == 's' ) {
					data_size =		table_info[column_index][E_TABLEDATA_SIZE];
					cache_get_field_content(row_index, table_info[column_index][E_TABLEDATA_DB_FIELD_NAME], dest_array[array_row][data_offset], db_handle, data_size);
				}
				column_index++;
			}
			count_items++;
		} else {
			printf("[TableHandler_DB_parseContent] invalid index found(%d) @ line=%d!", array_row, row_index);
			if( !is_indexed ) {
				break;
			}
		}
	}
	return count_items;
}

//	Parsing query results with selection parsing strategy (and saving auto-determined result).
stock TableHandler_DB_parseStrategy(
	const db_handle,
	&e_tablehandle_db_struct_mode:parsing_stategy,
	dest_array[][],
	const dest_size,
	const table_info[][e_table_model_struct_info],
	const table_info_size
) {
	if( parsing_stategy == E_TABLEDATA_RES_AUTODETECT ) {
		if(
			TableHandler_DB_IsSameStructRes(
				db_handle,
				table_info,
				table_info_size
			)
		) {
			parsing_stategy = E_TABLEDATA_RES_ISSAME;
		} else {
			parsing_stategy = E_TABLEDATA_RES_DIFFERENT;
		}
	}
	if( parsing_stategy == E_TABLEDATA_RES_ISSAME ) {
		TableHandler_DB_parseDirect(
			db_handle,
			dest_array,
			dest_size,
			table_info,
			table_info_size
		);
	} else {
		TableHandler_DB_parseContent(
			db_handle,
			dest_array,
			dest_size,
			table_info,
			table_info_size
		);
	}
}
