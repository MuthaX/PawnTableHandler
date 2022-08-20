#if(!defined __TABLEHANDLER_INCLUDED__ )
	#error The file PawnTableHandler_Main.inc have to be included before PawnTableHandler_Files.inc!
#endif

//	Reads structured table from file into dest_array.
//	Returns negative value if any serious problem found, otherwise number of loaded rows.
stock TableHandler_loadStructFile(
	const filepath[],							// Destination of file to load.
	const delimiter,							// Character used for split columns.
	dest_array[][],								// Destination array.
	const dest_size,							// Number of rows in destination array.
	const table_info[][e_table_model_struct_info],	// Structure of table.
	const table_info_size,						// Number of columns in table structure.
	const table_name[]							// Indentification name.
) {
	new
		error_counter = TableHandler_isInvalidStruct(table_info, table_info_size, table_name),
		File:f_handle,
		line_index = 0,
		count_items = -1	// error by default
	;
	#if !defined TABLEHANDLER_STRUCTTEST_DISABLE
	if( error_counter ) {
		printf("[TableLoader_fromFileStruct(%s)] cant load table from[%s] due to the problems in table structure (found %d errors)!", table_name, filepath, error_counter);
		return count_items;
	}
	if( table_info_size > TABLEHANDLER_MAX_COLUMNS ) {	// This procedure is using fixed size array for column boundaries in buffered string.
		printf("[TableLoader_fromFileStruct(%s)] cant load table from[%s] because it is too many columns(%d/%d)!", table_name, filepath, table_info_size, TABLEHANDLER_MAX_COLUMNS);
		return count_items;
	}
	if( TableHandler_isInvalidDelimiter(delimiter) ) {
		printf("[TableLoader_fromFileStruct(%s)] cant load table from[%s] because of invalid delimiter!", table_name, filepath);
		return count_items;
	}
	#endif
	if( (f_handle = fopen(filepath, io_read)) ) {
		static buffer_string[TABLEHANDLER_MAX_STRING_SIZE];
		new buffer_len, buffer_char, buffer_index,
			column_index,
			column_borders_at[TABLEHANDLER_MAX_COLUMNS][2],
			bool:was_content,
			data_type,
			data_offset,
			data_size,
			is_packed,
			put_in_row = -1,
			value_int, Float:value_float, column_starts_at
		;
		count_items = 0;
		while( (buffer_len = fread(f_handle, buffer_string, sizeof(buffer_string))) ) {
			line_index++;
			//	Step -1: Clear comments.
			buffer_index = 0;
			new commentary_pos;
			while( (commentary_pos = strfind(buffer_string, "#", false, buffer_index)) != -1 ) {
				// Search for first unescaped commentary symbol.
				if( !commentary_pos || buffer_string[commentary_pos - 1] != '\\' ) {
					buffer_string[commentary_pos] = EOS;
					buffer_len = commentary_pos;
					break;
				}
				buffer_index = commentary_pos + 1;
			}
			//	Step 0: Filtering lines.
			//	Possible to trim with some std procedure.
			for(buffer_index = buffer_len - 1; buffer_index > 0; buffer_index--) {
				buffer_char = buffer_string[buffer_index];
				if( buffer_char == '\n' || buffer_char == '\r' ) {
					buffer_len = buffer_index;
					buffer_string[buffer_index] = EOS;
				} else {
					break;
				}
			}
			if( buffer_len < 1 ) {
				continue;
			}
			//	Step 1: indexing column borders.
			was_content = false;
			column_index = 0;
			for(buffer_index = 0; buffer_index < buffer_len; ++buffer_index) {
				buffer_char = buffer_string[buffer_index];
				if( (buffer_char == delimiter) == was_content ) {
					if( was_content ) {
						buffer_string[buffer_index] = EOS;
					}
					column_borders_at[column_index][was_content] = buffer_index;
					column_index += _:was_content;
					was_content = !was_content;
				}
			}
			column_borders_at[column_index][1] = buffer_len;
			if( ++column_index != table_info_size ) {
				printf("[TableLoader_fromFileStruct(%s)] found invalid number in columns(n=%d ts=%d) @ line=%d!", table_name, column_index, table_info_size, line_index);
				continue;
			}
			//	Step 2: parse columns.
			for(column_index = 0; column_index < table_info_size; ++column_index) {
				data_type =			table_info[column_index][E_TABLEDATA_TYPE];
				data_offset =		table_info[column_index][E_TABLEDATA_OFFSET];
				data_size =			table_info[column_index][E_TABLEDATA_SIZE];
				is_packed =			table_info[column_index][E_TABLEDATA_OPTIONS] & 2;
				column_starts_at =	column_borders_at[column_index][0];
				if( data_type == '@' || data_type == 'd' ) {
					value_int = strval(buffer_string[column_starts_at]);
				} else if( data_type == 'f' ) {
					value_float = floatstr(buffer_string[column_starts_at]);
				} else if( data_type == 'h' ) {

				}
				if( column_index == 0 ) {
					if( data_type == '@' ) {
						put_in_row = value_int;
					} else {
						put_in_row++;
					}
				}
				if( put_in_row >= dest_size ) {
					printf("[TableLoader_fromFileStruct(%s)] too many items loaded or too high id found(%d) @ line=%d!", table_name, put_in_row, line_index);
					break;
				} else if( put_in_row < 0 ) {
					printf("[TableLoader_fromFileStruct(%s)] negative id found (%d) @ line=%d!", table_name, put_in_row, line_index);
					break;
				}

				if( data_type == 'd' || data_type == 'h' ) {
					dest_array[put_in_row][data_offset] = value_int;
				} else if( data_type == 'f' ) {
					dest_array[put_in_row][data_offset] = _:value_float;
				} else if( data_type == '@' ) {
					continue;
				} else {
					if( is_packed ) {
						buffer_string[column_borders_at[column_index][1]] = EOS;
						strpack(dest_array[put_in_row][data_offset], buffer_string[column_starts_at], data_size);
					} else {
						strmid(dest_array[put_in_row][data_offset], buffer_string, column_starts_at, column_borders_at[column_index][1], data_size);
					}
				}
			}
			count_items++;
		}
		fclose(f_handle);
	} else {
		printf("[TableLoader_fromFileStruct(%s)] cant open file (%s)!", table_name, filepath);
	}
	return count_items;
}

//	Reads structured table from file into dest_array.
//	There is no limitation in string size or number of columns at cost of low performance.
//	Returns negative value if any serious problem found, otherwise number of loaded rows.
stock TableHandler_loadLargeStruct(
	const filepath[],							// Destination of file to load.
	const delimiter,							// Character used for split columns.
	dest_array[][],								// Destination array.
	const dest_size,							// Number of rows in destination array.
	const table_info[][e_table_model_struct_info],	// Structure of table.
	const table_info_size,						// Number of columns in table structure.
	const bool:use_utf,
	const table_name[]							// Indentification name.
) {
	new
		error_counter = TableHandler_isInvalidStruct(table_info, table_info_size, table_name),
		File:f_handle,
		line_index = 1,
		count_items = -1	// error by default
	;
	#if !defined TABLEHANDLER_STRUCTTEST_DISABLE
	if( error_counter ) {
		printf("[TableLoader_fromFileLargeStruct(%s)] cant load table from[%s] due to the problems in table structure (found %d errors)!", table_name, filepath, error_counter);
		return count_items;
	}
	if( TableHandler_isInvalidDelimiter(delimiter) ) {
		printf("[TableLoader_fromFileLargeStruct(%s)] cant load table from[%s] because of invalid delimiter!", table_name, filepath);
		return count_items;
	}
	#endif
	if( (f_handle = fopen(filepath, io_read)) ) {
		new buffer_char,
			column_index = 0,
			was_content = 0,
			data_type,
			data_offset,
			data_size,
			is_packed,
			put_in_row = -1,
			value_int, Float:value_float,
			substring[32], substring_index
		;
		count_items = 0;
		while( (buffer_char = fgetchar(f_handle, 0, use_utf)) != EOF ) {
			if( buffer_char == '\n' || buffer_char == '\r' || buffer_char == delimiter ) {	// new line or separator.
				if( was_content ) {
					was_content = 0;
					if( ++substring_index >= data_size ) {
						substring_index = data_size - 1;
					}
					if( data_type != 's' ) {
						substring[substring_index] = EOS;
					}
					if( data_type == '@' || data_type == 'd' ) {
						value_int = strval(substring);
					} else if( data_type == 'f' ) {
						value_float = floatstr(substring);
					} else if( data_type == 'h' ) {

					}
					if( data_type == '@' ) {
						put_in_row = value_int;
					}
					if( put_in_row < dest_size && put_in_row >= 0 ) {
						if( column_index < table_info_size ) {
							if( data_type == 'd' || data_type == 'h' ) {
								dest_array[put_in_row][data_offset] = value_int;
							} else if( data_type == 'f' ) {
								dest_array[put_in_row][data_offset] = _:value_float;
							} else if( data_type == 's' ) {
								if( is_packed ) {
									dest_array[put_in_row][data_offset + (substring_index >> 2)] |= 0 << (substring_index & 3);
								} else {
									dest_array[put_in_row][data_offset + substring_index] = EOS;
								}
							}
						} else {
							printf("[TableLoader_fromFileLargeStruct(%s)] found invalid number in columns(n=%d ts=%d) @ line=%d!", table_name, column_index, table_info_size, line_index);
						}
					} else {
						printf("[TableLoader_fromFileLargeStruct(%s)] invalid row id (%d) found @ line=%d!", table_name, put_in_row, line_index);
					}
					if( buffer_char == delimiter ) {
						column_index++;
					}
				}
				if( buffer_char == '\n' ) {
					line_index++;
					column_index = 0;
					if( table_info[column_index][E_TABLEDATA_TYPE] != '@' ) {
						put_in_row++;
					}
				}
			} else {	// content symbol.
				if( was_content ) {
					substring_index++;
				} else {
					was_content = 1;
					substring_index = 0;
					data_type =			table_info[column_index][E_TABLEDATA_TYPE];
					data_offset =		table_info[column_index][E_TABLEDATA_OFFSET];
					is_packed =			table_info[column_index][E_TABLEDATA_OPTIONS] & 2;
					if( data_type == 's' ) {
						data_size =		table_info[column_index][E_TABLEDATA_SIZE];
					} else {
						data_size = sizeof(substring);
					}
				}
				if( substring_index >= data_size ) {	// truncate substring.
					substring_index = data_size - 1;
					buffer_char = EOS;
				}
				if( put_in_row >= 0 && put_in_row < dest_size ) {
					if( data_type == 's' ) {
						if( is_packed ) {
							dest_array[put_in_row][data_offset + (substring_index >> 2)] |= buffer_char << (substring_index & 3);
						} else {
							dest_array[put_in_row][data_offset + substring_index] = buffer_char;
						}
					} else {
						substring[substring_index] = buffer_char;
					}
				}
			}
		}
		fclose(f_handle);
	} else {
		printf("[TableLoader_fromFileLargeStruct] cant open file (%s)!", filepath);
	}
	return count_items;
}

//	Saves structured table into file from dest_array.
//	Returns negative value if any serious problem found, otherwise number of written rows.
stock TableHandler_saveStructFile(
	const filepath[],							// Destination of file to save.
	const delimiter,							// Character used for split columns.
	src_array[][],								// Source array.
	const src_size,								// Number of rows in source array.
	const table_info[][e_table_model_struct_info],	// Structure of table.
	const table_info_size,						// Number of columns in table structure.
	bool:use_utf,
	const table_name[]							// Indentification name.
) {
	new
		error_counter = TableHandler_isInvalidStruct(table_info, table_info_size, table_name),
		File:f_handle,
		count_items = -1
	;
	#if !defined TABLEHANDLER_STRUCTTEST_DISABLE
	if( error_counter ) {
		printf("[TableSaver_toFileStruct(%s)] cant save table due to the problems in table structure (found %d errors)!", table_name, error_counter);
		return count_items;
	}
	if( TableHandler_isInvalidDelimiter(delimiter) ) {
		printf("[TableLoader_fromFileStruct(%s)] cant load table from[%s] because of invalid delimiter!", table_name, filepath);
		return count_items;
	}
	#endif
	if( (f_handle = fopen(filepath, io_write)) ) {
		new column_index,
			data_type,
			data_offset,
			data_precision,
			data_size,
			is_packed,
			item_index,
			symbol,
			substring[32]
		;
		count_items = 0;
		for(item_index = 0; item_index <  src_size; ++item_index) {
			for(column_index = 0; column_index < table_info_size; ++column_index) {
				data_type =			table_info[column_index][E_TABLEDATA_TYPE];
				data_offset =		table_info[column_index][E_TABLEDATA_OFFSET];
				data_precision =	table_info[column_index][E_TABLEDATA_PRECISION];
				data_size =			table_info[column_index][E_TABLEDATA_SIZE];
				is_packed =			table_info[column_index][E_TABLEDATA_OPTIONS] & 2;
				if( data_type == '@' ) {
					format(substring, sizeof(substring), "%0*d", data_size, item_index);
					fwrite(f_handle, substring);
				} else if( data_type == 'd' ) {
					format(substring, sizeof(substring), "%0*d", data_size, src_array[item_index][data_offset]);
					fwrite(f_handle, substring);
				} else if( data_type == 'f' ) {
					format(substring, sizeof(substring), "%0*.*f", data_size, data_precision, src_array[item_index][data_offset]);
					fwrite(f_handle, substring);
				} else if( data_type == 'h' ) {
					format(substring, sizeof(substring), "%0*x", data_size, src_array[item_index][data_offset]);
					fwrite(f_handle, substring);
				} else if( data_type == 's' ) {
					if( !src_array[item_index][data_offset] ) {	// at zero index.
						printf("[TableSaver_toFileStruct(%s)] NULL string found at row=%d column=%d!", table_name, item_index, column_index);
						src_array[item_index][data_offset] = '?';
						src_array[item_index][data_offset + 1] = EOS;
					}
					if( is_packed ) {
						for(new substring_index = 0; substring_index < data_size; ++substring_index) {
							symbol = src_array[item_index][data_offset + substring_index / 4] >> (8 * (substring_index & 3));
							fputchar(f_handle, symbol & 0xFF, use_utf);
						}
					} else {
						if( use_utf ) {
							fwrite(f_handle, src_array[item_index][data_offset]);
						} else {
							for(new substring_index = 0; substring_index < data_size; ++substring_index) {
								symbol = src_array[item_index][data_offset + substring_index];
								if( symbol ) {
									fputchar(f_handle, symbol, false);
								} else {
									break;
								}
							}	
						}
					}
				}
				if( column_index + 1 < table_info_size ) {
					substring[0] = delimiter;
					substring[1] = EOS;
					fwrite(f_handle, substring);
				}
			}
			fwrite(f_handle, "\n");
			count_items++;
		}
		fclose(f_handle);
	} else {
		printf("[TableSaver_toFileStruct(%s)] cant open file (%s)!", table_name, filepath);
	}
	return count_items;
}
