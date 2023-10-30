#if !defined __TABLEHANDLER_DB_DEFS__
#define __TABLEHANDLER_DB_DEFS__

#if defined cache_get_field_type	//	R40
#define PTH_MYSQL_GET_ROWCOUNT(%0,%1)				cache_get_row_count(%0)
#define PTH_MYSQL_FIELDID_GETINT(%0,%1,%2,%3)		cache_get_value_index_int(%0,%1,%2)
#define PTH_MYSQL_FIELDNAME_GETINT(%0,%1,%2,%3)		cache_get_value_name_int(%0,%1,%2)
#define PTH_MYSQL_FIELDID_GETFLOAT(%0,%1,%2,%3)		cache_get_value_index_float(%0,%1,Float:%2)
#define PTH_MYSQL_FIELDNAME_GETFLOAT(%0,%1,%2,%3)	cache_get_value_name_float(%0,%1,Float:%2)
#define PTH_MYSQL_FIELDID_GETSTR(%0,%1,%2,%3,%4)	cache_get_value_index(%0,%1,%2,%4)
#define PTH_MYSQL_FIELDNAME_GETSTR(%0,%1,%2,%3,%4)	cache_get_value_name(%0,%1,%2,%4)
#else
#define PTH_MYSQL_GET_ROWCOUNT(%0,%1)				(%0)=cache_get_row_count(%1)
#define PTH_MYSQL_FIELDID_GETINT(%0,%1,%2,%3)		(%2)=cache_get_row_int(%0,%1,%3)
#define PTH_MYSQL_FIELDNAME_GETINT(%0,%1,%2,%3)		(%2)=cache_get_field_content_int(%0,%1,%3)
#define PTH_MYSQL_FIELDID_GETFLOAT(%0,%1,%2,%3)		(%2)=_:cache_get_row_float(%0,%1,%3)
#define PTH_MYSQL_FIELDNAME_GETFLOAT(%0,%1,%2,%3)	(%2)=_:cache_get_field_content_float(%0,%1,%3)
#define PTH_MYSQL_FIELDID_GETSTR(%0,%1,%2,%3,%4)	cache_get_row(%0,%1,%2,%3,%4)
#define PTH_MYSQL_FIELDNAME_GETSTR(%0,%1,%2,%3,%4)	cache_get_field_content(%0,%1,%2,%3,%4)
#endif

#endif
