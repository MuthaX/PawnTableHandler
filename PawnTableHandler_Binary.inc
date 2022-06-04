#if(!defined __TABLEHANDLER_INCLUDED__ )
	#error The file PawnTableHandler_Main.inc have to be included before PawnTableHandler_Files.inc!
#endif

//	Reads table from binary file by its handler.
//	Returns number of loaded items.
stock TableHandler_Bin_read(
	const File:f_handle,
	dest_array[][],
	const rows_to_read,
	const table_info[][e_table_model_struct_info],	// Structure of table.
	const table_info_size							// Number of columns in table structure.
) {
	new
		error_counter = TableHandler_isInvalidStruct(table_info, table_info_size),
		rows_number, row_index, column_index,
		cell_value, byte_value,
		data_type, data_bytes, data_size, data_offset, data_options,
		sub_index,
		bool:option_utf, option_pack,
		put_in_row = -1,
		count_items = 0
	;
	#if !defined TABLEHANDLER_STRUCTTEST_DISABLE
	if( error_counter ) {
		printf("[TableHandler_Bin_read] cant load table from[%s] due to the problems in table structure (found %d errors)!", error_counter);
		return -1;
	}
	#endif
	if( rows_to_read < 0 ) {
		rows_number = cellmax;
	}
	for(row_index = 0; row_index < rows_number; ++row_index) {
		for(column_index = 0; column_index < table_info_size; ++column_index) {
			data_type =		table_info[column_index][E_TABLEDATA_TYPE];
			data_offset =	table_info[column_index][E_TABLEDATA_OFFSET];
			if( data_type == 's' ) {
				data_size =		table_info[column_index][E_TABLEDATA_SIZE];
				data_bytes =	table_info[column_index][E_TABLEDATA_BINSYMBOLS];
				data_options =	table_info[column_index][E_TABLEDATA_OPTIONS];
				option_utf =	bool:(data_options & E_TABLEHANDLER_OPTION_UTF),
				option_pack =	data_options & E_TABLEHANDLER_OPTION_PACK
				for(sub_index = 0; sub_index < data_size; ++sub_index) {
					dest_array[put_in_row][data_offset + sub_index] = 0;					
				}
				for(sub_index = 0; sub_index < data_bytes; ++sub_index) {
					byte_value = (fgetchar(f_handle, 0, option_utf));
					if( byte_value == EOF ) {
						return count_items;
					}
					if( option_pack ) {
						dest_array[put_in_row][data_offset + sub_index] |= (byte_value << (8 * (sub_index & 3)));
					} else {
						dest_array[put_in_row][data_offset + sub_index] = byte_value;
					}
				}
			} else {
				cell_value = 0;
				for(sub_index = 0; sub_index < data_bytes; ++sub_index) {
					byte_value = (fgetchar(f_handle, 0, false) << (8 * sub_index));
					if( byte_value == EOF ) {
						return count_items;
					}
					cell_value |= byte_value;
				}
				if( !column_index ) {
					if( data_type == '@' ) {
						put_in_row = cell_value;
					} else {
						put_in_row++;
					}
				}
				dest_array[put_in_row][data_offset] = cell_value;
			}
		}
		++count_items;
	}
	return count_items;
}
//	Saves table from binary file by its handler.
stock TableHandler_Bin_write(
	const File:f_handle,
	const source_array[][],
	const rows_number,
	const table_info[][e_table_model_struct_info],	// Structure of table.
	const table_info_size							// Number of columns in table structure.
) {
	new
		error_counter = TableHandler_isInvalidStruct(table_info, table_info_size),
		row_index, column_index,
		cell_value, byte_value,
		data_type, data_bytes, data_size, data_offset, data_options,
		sub_index,
		bool:option_utf, option_pack
	;
	#if !defined TABLEHANDLER_STRUCTTEST_DISABLE
	if( error_counter ) {
		printf("[TableHandler_Bin_write] cant load table from[%s] due to the problems in table structure (found %d errors)!", error_counter);
		return -1;
	}
	for(row_index = 0; row_index < rows_number; ++row_index) {
		for(column_index = 0; column_index < table_info_size; ++column_index) {
			data_type =		table_info[column_index][E_TABLEDATA_TYPE];
			data_offset =	table_info[column_index][E_TABLEDATA_OFFSET];
			data_bytes =	table_info[column_index][E_TABLEDATA_BINSYMBOLS];
			if( data_type == 's' ) {
				data_size =		table_info[column_index][E_TABLEDATA_SIZE];
				data_options =	table_info[column_index][E_TABLEDATA_OPTIONS];
				option_utf =	bool:(data_options & E_TABLEHANDLER_OPTION_UTF);
				option_pack =	data_options & E_TABLEHANDLER_OPTION_PACK;
				if( option_pack ) {
					fblockwrite(f_handle, source_array[row_index][data_offset], data_size);
				} else {
					for(sub_index = 0; sub_index < data_size; ++sub_index) {
						fputchar(f_handle, source_array[row_index][data_offset + sub_index], option_utf);
					}
				}
			} else {
				if( data_type == '@' ) {
					cell_value = row_index;
				} else {
					cell_value = source_array[row_index][data_offset];
				}
				for(sub_index = 0; sub_index < data_bytes; ++sub_index) {
					byte_value = (cell_value >> (8 * sub_index)) & 0xFF;
					fputchar(f_handle, byte_value, false);
				}
				cell_value = 0;
			}
		}
	}
	#endif
}
