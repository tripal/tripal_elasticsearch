<?php

/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 1/23/17
 * Time: 5:20 PM
 */
class GetTableList
{
    protected $public_schema = 'public';

    protected $chado_schema = 'chado';

    public function get_chado_tables()
    {

        // get table list from the public schema
        $sql_public_tables = "SELECT table_name FROM information_schema.tables WHERE table_schema = $this->public_schema ORDER BY table_name;";
        $public_tables_query = db_query($sql_public_tables);
        foreach ($public_tables_query as $record) {
            $table = $record->table_name;
            $public_tables[$table] = $table;
        }

        return $public_tables;
    }

    public function get_public_tables()
    {

        // get table list from the chado schema
        $sql_chado_tables = "SELECT table_name FROM information_schema.tables WHERE table_schema = $this->chado_schema ORDER BY table_name;";
        $chado_tables_query = db_query($sql_chado_tables);
        foreach ($chado_tables_query as $record) {
            $table = 'chado.' . $record->table_name;
            $chado_tables[$table] = $table;
        }

        return $chado_tables;

    }

    public function get_public_and_chado_tables()
    {

        $public_and_chado_tables = $this->get_public_tables() + $this->get_chado_tables();

        return $public_and_chado_tables;

    }

    public function get_table_columns()
    {

        $table_columns = array();
        $sql_public_table = "SELECT column_name FROM information_schema.columns WHERE table_schema = $this->public_schema AND table_name = :selected_table;";
        $sql_chado_table = "SELECT column_name FROM information_schema.columns WHERE table_schema = $this->chado_schema AND table_name = :selected_table;";
        if (preg_match('/^chado\./', $table_name)) {
            $table_name = preg_replace('/^chado\./', '', $table_name);
            $query = db_query(
                $sql_chado_table,
                array(':selected_table' => $table_name)
            );
            foreach ($query as $record) {
                $field = $record->column_name;
                $table_columns[$field] = $field;
            }
        } else {
            $query = db_query(
                $sql_public_table,
                array(':selected_table' => $table_name)
            );
            foreach ($query as $record) {
                $field = $record->column_name;
                $table_columns[$field] = $field;
            }
        }

        return $table_columns;

    }

}