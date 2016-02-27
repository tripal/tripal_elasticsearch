<?php



/**
 * Define a function to sort two dimensional array by values
 */
function sort_2d_array_by_value($arr, $field, $sort){
	$sorted_a = array();
	foreach($arr as $k=>$v){
		// create an array to store the field values
		$field_array[$k] = $v[$field]; 
	}
	
	if($sort == 'asc'){
		asort($field_array);
	}
	else{
		arsort($field_array);
	}
	
	foreach(array_keys($field_array) as $k){
		$sorted_a[] = $arr[$k];
	}
	
	return $sorted_a;
}
//================End of sort_2d_array_by_value() =====================


/**
 * Define a function to get the primary key of a table
 */
function get_primary_key($table_name){
	if(in_array($table_name, get_chado_table_list())){
		$table = 'chado.'.$table_name;
		$primary_key_sql = 'SELECT a.attname
							FROM   pg_index i
							JOIN   pg_attribute a ON a.attrelid = i.indrelid
                     					AND a.attnum = ANY(i.indkey)
							WHERE  i.indrelid = \''.$table.'\'::regclass
							AND    i.indisprimary;';

		$primary_key = db_query($primary_key_sql)->fetchAssoc();
	}else{
		$table = $table_name;
			$primary_key_sql = 'SELECT a.attname
								FROM   pg_index i
								JOIN   pg_attribute a ON a.attrelid = i.indrelid
                     						AND a.attnum = ANY(i.indkey)
								WHERE  i.indrelid = \''.$table.'\'::regclass
								AND    i.indisprimary;';

			$primary_key = db_query($primary_key_sql)->fetchAssoc();
	}


	if(is_array($primary_key)){
		$primary_key = implode($primary_key);
	}

	return $primary_key;

}//============== End of primary key function ================================






/**
 * This function return an array containing tables from the chado schema
 */
function get_chado_table_list() {

    $sql_table_list = "SELECT table_name FROM information_schema.tables WHERE table_schema = 'chado' ORDER BY table_name;";
    $result_table = db_query($sql_table_list);
    $input_table = $result_table->fetchAll();


    $table_list = array();
    $i = 0;
    foreach ($input_table as $value) {
        $table_list[$i] = $value->table_name;
        $i++;
    }

    return $table_list;
}


/**
 * This function takes a table name and return all the column names
 */
function get_column_list($table_name) {
    $sql_column_list = "SELECT column_name FROM information_schema.columns WHERE (table_schema = 'public'OR table_schema = 'chado') AND table_name = :selected_table;";
    $result_column = db_query($sql_column_list, array(':selected_table' => $table_name));
    $input_column = $result_column->fetchAll();
    $column_list = array();

    $k = 0;
    foreach($input_column as $value) {
        $column_list[$k] = $value->column_name;
        $k++;
    }

    return $column_list;
}

/**
 * This function return an array containing a list of table names from the public OR chado schema.
 * All the tables are from the chado schema or public schema.
 */
function get_table_list() {

    $sql_table_list = "SELECT table_name FROM information_schema.tables WHERE (table_schema = 'public' OR table_schema = 'chado') ORDER BY table_name;";
    $result_table = db_query($sql_table_list);
    $input_table = $result_table->fetchAll();


    $table_list = array();
    $i = 0;
    foreach ($input_table as $value) {
        $table_list[$i] = $value->table_name;
        $i++;
    }

    return $table_list;
}



/**
 * This function transforms an object to an array resursively.<br/>
 */
function objectToArray($d) {
    if (is_object($d)) {
        // Gets the properties of the given object
        // with get_object_vars function
        $d = get_object_vars($d);
    }

    if (is_array($d)) {
        /*
        * Return array converted to object
        * Using __FUNCTION__ (Magic constant)
        * for recursive call
        */
        return array_map(__FUNCTION__, $d);
    }
    else {
        // Return array
        return $d;
    }
}

/**
 * This function transform a nested array to a flattened array.
 */
function flatten($arr, $prefix = '') {
        $out = array();
        foreach ($arr as $key => $value) {
                $key = (!strlen($prefix)) ? $key : "{$prefix}_$key";
                if (is_array($value)) {
                        $out += flatten($value, $key);
                } else {
                        $out[$key] = $value;
                }
        }

        return $out;
}
