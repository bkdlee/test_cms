<?php
    require_once '../src/includes/classes/pg_field_class.inc.php';

    $db_trans='';
    $db_link='';
    $db_misc_time = 0;
    $db_query_count = $db_query_time = 0;
    $db_fetch_count = $db_fetch_time = 0;
    $db_error_handler=null;

    $db_is_in_transaction = false;


    if (!function_exists('mysqli_connect')) die("Mysql support not enabled in php.");

    function db_vacuum($tables) {
        if (!is_array($tables)) $tables=array($tables);

        foreach ($tables as $index => $table_name) {
            db_query("VACUUM ANALYZE \"{$table_name}\"");
        }
        return true;

    }

    function db_get_all_sequences() {
        $sql = "SELECT  c.relname as \"table\",
                        a.attname AS \"field\",
                        pg_get_serial_sequence(c.relname, a.attname) as \"name\"
                FROM    pg_attribute a
                        INNER JOIN pg_class c ON c.oid = a.attrelid
                        INNER JOIN pg_type t ON t.oid = a.atttypid
                        LEFT OUTER JOIN pg_attrdef d ON c.oid = d.adrelid AND a.attnum = d.adnum
                WHERE   a.attnum > 0
                        AND d.adsrc like 'nextval(%'
                ORDER BY a.attnum";
        $query = db_query($sql);
        $return=array();
        while ($row =db_fetch_array($query, 'assoc')) {
            if (!empty($row['name'])) {
               $return[]=$row;
            }
        }
        return $return;
    }

    function db_create_fks_and_indexes($table_name, $fks_and_indexes) {
        foreach ($fks_and_indexes['indexes'] as $index => $row){
            db_query($row['index_src']);
        }
        foreach ($fks_and_indexes['foreign_keys'] as $index => $row){
            db_query($row['fk_src']);
        }
        return true;

    }
    /**
     * Drops any FK and indexes so bulk-import runs much quicker
     * @return array of indexes and fks which can be used with db_create_fks_and_indexes
     */
    function db_drop_fks_and_indexes($table_name) {
        global $db_is_in_transaction;
        if ($db_is_in_transaction !== true) {
            throw new Exception("Database is not in a transaction - aborting drop fks + indexes.");
        }
        $return=array(
            'indexes'=>array(),
            'foreign_keys'=>array()
        );

        $sql = "SELECT  c.relname as index_name,
                        pg_catalog.pg_get_indexdef(i.indexrelid) as index_src
                FROM    pg_catalog.pg_class c
                        JOIN pg_catalog.pg_index i ON i.indexrelid = c.oid
                        JOIN pg_catalog.pg_class c2 ON i.indrelid = c2.oid
                WHERE   c2.relname = '{$table_name}'
                        AND c.relkind = 'i'
                        AND i.indisprimary = 'f'
                ORDER BY 1,2";

        $query = db_query($sql);
        $drop_sql = '';

        while ($row = db_fetch_array($query, 'assoc')) {
            $return['indexes'][]=$row;
            $drop_sql .= "DROP INDEX \"{$row['index_name']}\";\n";
        }

        $sql = "SELECT  i.conname as fk_name,
                        pg_catalog.pg_get_constraintdef(i.oid) as fk_src
                FROM    pg_catalog.pg_class c
                        JOIN pg_catalog.pg_constraint i ON i.conrelid = c.oid
                        JOIN pg_catalog.pg_class c2 ON i.conrelid = c2.oid
                WHERE   c.relname = '{$table_name}'
                        AND i.contype = 'f'
                ORDER BY 1, 2";
        $query = db_query($sql);
        while ($row = db_fetch_array($query, 'assoc')) {
            $row['fk_src'] = "ALTER TABLE \"{$table_name}\" ADD {$row['fk_src']}";
            $return['foreign_keys'][]=$row;
            $drop_sql .= "ALTER TABLE \"{$table_name}\" DROP CONSTRAINT \"{$row['fk_name']}\";\n";
        }

        if ($drop_sql !== '') {
            db_query($drop_sql);
        }

        return $return;

    }
    function set_sql_error_handler($function) {
        global $db_error_handler;
        $db_error_handler = $function;
    }

    function db_error($connection, $error, $data=null) {
        global $db_error_handler, $db_link;
        $connection = ((is_null($connection)) ? $db_link : $connection);
        $last_message = pg_last_error($connection);
        if (is_null($db_error_handler)) {
            if (defined('RPC_MODE')) {
                global $rpc_xml;
                if (isset($rpc_xml)) {
                    $rpc_xml->error("Unexpected database error: [{$error}]", $data);
                } else {
                    die("Unexpected database error: [{$error}]<br><pre>" . print_r($data) . "</pre>");
                }
            } else {
                die_err('SQL SERVER ERROR<br><p>' . $error . '</p>Last Message: ' . $last_message . '.<br>@@ERROR: ' . get_sql_error(), $data);
            }
        } else {
            call_user_func($db_error_handler, $connection, $error, $data, $last_message);
        }
    }

    function db_delete($table, $where) {
        if ($table=='') db_error(null, "Table name not set");
        if ($where=='') db_error(null, "WHERE condition not set");
        $sql = "DELETE FROM \"{$table}\" WHERE {$where}";
        return db_query($sql);
    }

    function db_identify($connection=null) {
        $sql = "SELECT  @@SERVERNAME as server_name,
                        DB_NAME() as db_name,
                        NOW() as db_time";
        $query = db_query($sql, $connection);
        return db_fetch_array($query, 'assoc');
    }

    function db_get_table_names($connection=null) {
        $sql = "SELECT  c.relname
                FROM    pg_class c
                        INNER JOIN pg_namespace n on n.oid = c.relnamespace
                WHERE   c.relkind = 'r'
                        AND n.nspname = 'public'
                ORDER BY c.relname";
        $query=db_query($sql, $connection);
        while ($row = db_fetch_array($query, 'assoc')) {
            $return[]=$row['relname'];
        }
        return $return;
    }


    function db_fetch_blank_row($query) {
        $field_count = pg_num_fields($query);
        $fields=array();
        for ($i=0;$i<$field_count;$i++) {
            $fields[pg_field_name($query, $i)] = '';
        }
        return $fields;
    }

    function db_get_tables($connection=null) {
        $sql = "SELECT  c.relname
                FROM    pg_class c
                        INNER JOIN pg_namespace n on n.oid = c.relnamespace
                WHERE   c.relkind = 'r'
                        AND n.nspname = 'public'
                ORDER BY c.relname";
        $query=db_query($sql, $connection);
        while ($row = db_fetch_array($query, 'assoc')) {
            $tables[$row['name']]=array(
                'fields'    => db_get_table_fields($row['name'],$connection),
                'pk'        => db_get_table_primary_keys($row['name'],$connection),
                'unique'    => db_get_table_unique_indexes($row['name'],$connection),
                'fk'        => db_get_foreign_keys($row['name'], $connection)
            );
        }
        return $tables;
    }

    function db_get_foreign_keys($table_name, $connection=null) {
        static $cache=null;
        if (!is_array($cache)) {
            $sql = "SELECT  tc.constraint_name,
                            tc.table_name,
                            kcu.column_name,
                            ccu.table_name AS f_table,
                            ccu.column_name AS f_field,
                            rc.update_rule as \"update\",
                            rc.delete_rule as \"delete\"
                    FROM    information_schema.table_constraints AS tc
                            JOIN information_schema.key_column_usage AS kcu ON tc.constraint_name = kcu.constraint_name
                            JOIN information_schema.constraint_column_usage AS ccu ON ccu.constraint_name = tc.constraint_name
                            JOIN information_schema.referential_constraints rc ON ccu.constraint_name = rc.constraint_name
                    WHERE   constraint_type = 'FOREIGN KEY'";

            $query = db_query($sql);
            $cache=array();
            while ($row = db_fetch_array($query, 'assoc')) {
                $t_name     = $row['table_name'];
                $name       = $row['constraint_name'];
                if (!isset($cache[$t_name]))        $cache[$t_name] = array();
                if (!isset($cache[$t_name][$name])) $cache[$t_name][$name] = array();
                unset($row['constraint_name']);
                unset($row['table_name']);
                $cache[$t_name][$name][]=$row;
            }
        }
        if (isset($cache[$table_name])) return $cache[$table_name];
        return array();

    }
    function db_get_table_fields($table_name, $connection=null) {
        static $cache=null;
        if (!is_array($cache)) {
            $sql = "SELECT  o.name as table_name,
                            c.name   as \"field\",
                            CASE WHEN autoval IS NULL THEN t.name ELSE t.name + ' IDENTITY' END as \"type\",
                            c.length as \"length\",
                            c.xprec  as \"precision\",
                            c.xscale as \"scale\",
                            CASE WHEN c.isnullable = 1 THEN 'NULL' ELSE 'NOT NULL' END as \"nullable\",
                            d.text as \"default\"
                    FROM    syscolumns c
                            INNER JOIN sysobjects o ON o.id = c.id
                            INNER JOIN systypes t ON c.xtype = t.xtype
                            LEFT OUTER JOIN syscomments d ON c.cdefault = d.id
                    ORDER BY c.colid
                    ";
            $query = db_query($sql, $connection);
            $cache=array();
            while ($row = db_fetch_array($query, 'assoc')) {
                $field  = $row['field'];
                $t_name = $row['table_name'];
                if (strpos($row['default'], "nextval('") !== false) $row['default'] = '';
                unset($row['field']);
                unset($row['table_name']);
                if (!isset($cache[$t_name]))        $cache[$t_name] = array();
                $cache[$t_name][$field] = $row;
            }
        }
        if (isset($cache[$table_name])) return $cache[$table_name];
        return array();
    }

    function db_get_table_primary_keys($table_name, $connection=null) {
        static $cache=null;
        if (!is_array($cache)) {
            $sql = "SELECT  KU.TABLE_NAME as table_name,
                            KU.CONSTRAINT_NAME as constraint_name,
                            KU.COLUMN_NAME as field_name
                    FROM    INFORMATION_SCHEMA.TABLE_CONSTRAINTS AS TC
                            INNER JOIN INFORMATION_SCHEMA.KEY_COLUMN_USAGE AS KU
                                ON TC.CONSTRAINT_TYPE = 'PRIMARY KEY'
                                AND TC.CONSTRAINT_NAME = KU.CONSTRAINT_NAME
                    ORDER BY KU.TABLE_NAME, KU.ORDINAL_POSITION";
            $query = db_query($sql);
            $cache=array();
            while ($row = db_fetch_array($query, 'assoc')) {
                if (!isset($cache[$row['table_name']])) $cache[$row['table_name']] = array();
                if (!isset($cache[$row['table_name']][$row['constraint_name']])) $cache[$row['table_name']][$row['constraint_name']] = array();
                $cache[$row['table_name']][$row['constraint_name']][]=$row['field_name'];
            }
        }
        if (isset($cache[$table_name])) return $cache[$table_name];
        return array();
    }

    function db_get_table_unique_indexes($table_name, $connection=null) {
        static $cache=null;
        if (!is_array($cache)) {
            $sql = "SELECT  KU.TABLE_NAME as table_name,
                            KU.CONSTRAINT_NAME as constraint_name,
                            KU.COLUMN_NAME as field_name
                    FROM    INFORMATION_SCHEMA.TABLE_CONSTRAINTS AS TC
                            INNER JOIN INFORMATION_SCHEMA.KEY_COLUMN_USAGE AS KU
                                ON TC.CONSTRAINT_TYPE = 'UNIQUE'
                                AND TC.CONSTRAINT_NAME = KU.CONSTRAINT_NAME
                    ORDER BY KU.TABLE_NAME, KU.ORDINAL_POSITION";
            $query = db_query($sql);
            $cache=array();
            while ($row = db_fetch_array($query, 'assoc')) {
                if (!isset($cache[$row['table_name']])) $cache[$row['table_name']] = array();
                if (!isset($cache[$row['table_name']][$row['constraint_name']])) $cache[$row['table_name']][$row['constraint_name']] = array();
                $cache[$row['table_name']][$row['constraint_name']][]=$row['field_name'] ;
            }
        }
        if (isset($cache[$table_name])) return $cache[$table_name];
        return array();
    }


    function db_get_table_indexes($table_name, $connection=null) {
        static $cache=null;
        if (!is_array($cache)) {
            $sql = "SELECT  o.name as table_name,
                            i.name as index_name,
                            c.name as field_name
                    FROM    sysindexes i
                            INNER JOIN sysobjects o ON i.id = o.id
                            INNER JOIN sysindexkeys k ON i.id = k.id AND i.indid = k.indid
                            INNER JOIN syscolumns c ON k.id = c.id AND k.colid = c.colid
                    WHERE   i.indid > 0
                            AND i.indid < 255
                            AND INDEXPROPERTY(i.id, i.name, 'IsStatistics') = 0
                            AND INDEXPROPERTY(I.id, I.name, 'IsHypothetical') = 0
                            AND i.name NOT IN (
                                SELECT  o2.name
                                FROM    sysobjects o2
                                WHERE   o2.parent_obj = i.id
                                        AND OBJECTPROPERTY(o2.id, 'isConstraint') = 1
                            )";
            $query = db_query($sql, $connection);
            $cache=array();
            while ($row = db_fetch_array($query, 'assoc')) {
                if (!isset($cache[$row['table_name']])) $cache[$row['table_name']] = array();
                if (!isset($cache[$row['table_name']][$row['index_name']])) $cache[$row['table_name']][$row['index_name']] = array();
                $cache[$row['table_name']][$row['index_name']][]=$row['field_name'];
            }

        }
        if (isset($cache[$table_name])) return $cache[$table_name];
        return array();
    }

    function db_dump($sql, $connection=null) {
        echo "<pre>" . $sql . "</pre>";

        $query = db_query($sql, $connection);
        if (!db_num_rows($query)) {
            echo "<b>No rows</b>";
        } else {
            echo "<h1>Row Count: " . db_num_rows($query) . "</h1>";
            echo '<table border="1">';
            $row_count=0;
            while (($row = db_fetch_array($query, 'assoc')) && ($row_count < 100)) {
                $row_count++;
                if ($row_count == 1) {
                    echo '<thead>';
                    echo '<tr>';
                    echo '<th>#</th>';
                    foreach ($row as $field => $v) {
                        echo '<th>' . $field . '</th>';
                    }
                    echo '</tr>';
                    echo '</thead>';
                }

                echo '<tr>';
                echo '<td>' . $row_count . '</td>';
                foreach ($row as $field => $v) {
                    echo '<td>' . $v . '</td>';
                }
                echo '</tr>';
            }
            echo "</table>";
        }

        die("done");


    }

    function db_set_dbname($client, $country_code) {
        global $config;
        $config->sql_database = strtolower(((SANDBOX == 'T') ? 'sandbox' : 'ac') . '_' . $client . '_' . $country_code);
    }

    function db_get_unique_id($table, $field, $connection=null) {
        $sql = "SELECT  MAX($field) as return_value
                FROM    \"$table\"";
        $query = db_query($sql, $connection);
        if (!db_num_rows($query)) return 1;
        $result = db_fetch_array($query, 'assoc');
        return (intval($result['return_value']) + 1);
    }

    function db_update_text($table, $field, $where, &$text, $append=false, $connection=null) {
        global $config;
        if ($append==false) {
            db_query("UPDATE \"{$table}\" SET \"{$field}\" = NULL WHERE {$where}", $connection);
        }

        $str_pos=0;
        $buffer_len = 1024;
        db_query("EXEC sp_dboption '{$config->sql_database}', 'select into/bulkcopy', 'true'", $connection);

        while ($str_pos < strlen($text)) {
            $buffer = substr($text, $str_pos, $buffer_len);

            $buffer = db_string($buffer);
            // UPDATETEXT table.field ptr offset chars_to_delete 'text to append'
            // chars_to_delete is used delete characters after the offset, set to null if no delete needed

            $sql = "DECLARE @ptrval binary(16)

                    SELECT  @ptrval = TEXTPTR($field)
                    FROM    \"$table\"
                    WHERE   $where

                    UPDATETEXT \"{$table}\".$field @ptrval $str_pos NULL $buffer";
            db_query($sql, $connection);
            $str_pos += $buffer_len;

        }
        db_query("EXEC sp_dboption '{$config->sql_database}', 'select into/bulkcopy', 'false'", $connection);

    }

    function _db_time() {
        return microtime(true);
    }

    function _db_time_diff($start) {
        return (microtime(true)-$start);
    }

    function db_begin($id='', $connection=null) {
        global $application, $db_trans, $db_misc_time, $db_is_in_transaction;

        if ($db_is_in_transaction === true) {
            if ($application->show_errors===true) {
                throw new Exception("Database transaction already started (db_begin called twice without db_commit or db_abort called)");
            }
        }
        $db_is_in_transaction = true;

        $start=_db_time();
        db_query("BEGIN", $connection);
        $db_misc_time += _db_time_diff($start);
    }

    function db_commit($id='', $connection = null) {
        global $db_trans,  $db_misc_time, $db_is_in_transaction, $application;
        $start=_db_time();
        $db_trans='';

        if ($db_is_in_transaction !== true) {
            if ($application->show_errors===true) {
                throw new Exception("Database transaction committed without being started (db_commit called without db_begin called, or db_commit called twice?)");
            }
        }
        $db_is_in_transaction = false;

        db_query("COMMIT", $connection);

        $db_misc_time += _db_time_diff($start);
    }

    function db_abort($id='', $connection=null) {
        global $db_trans,  $db_misc_time, $db_is_in_transaction, $application;
        if ($db_trans == '') return false;
        $start=_db_time();
        if ($db_is_in_transaction !== true) {
            if ($application->show_errors===true) {
                throw new Exception("Database transaction aborted without being started (db_abort called without db_begin called, or db_abort called twice?)");
            }
        }
        $db_is_in_transaction = false;

        db_query("ROLLBACK", $connection); // no quotes
        $db_misc_time += _db_time_diff($start);
    }

    function db_try_connect() {
        global $db_link, $config, $db_misc_time;
        $start=_db_time();
        if (!is_resource($db_link)) {
            $connection_string="host={$config->db_server} dbname={$config->db_name} user={$config->db_user} password={$config->db_pass}";
            $db_link = pg_connect($connection_string);
            if (!is_resource($db_link)) db_error(null, 'unable to connect to database server');
        }
        $db_misc_time += _db_time_diff($start);

        return true;

    }

    function db_connect() {
        global $db_link, $config, $db_misc_time;
        $start=_db_time();

        $connection_string="host={$config->db_server} dbname={$config->db_name} user={$config->db_user} password={$config->db_pass} port={$config->db_port}";
        $db_link = pg_connect($connection_string);
		
		
		
		
        if (!is_resource($db_link)) db_error(null, 'unable to connect to database server');

        $db_misc_time += _db_time_diff($start);

        return true;

    }
    function db_seek($result, $offset) {
        pg_result_seek($result, $offset);
    }

    function db_fetch_page($result, $first_record, $number_of_records) {

        if (!is_resource($result)) db_error(null, 'query passed is not a valid resource - run db_query first.', $result);
        // start from record 1
        if ($first_record < 1) $first_record == 1;

        $rows = db_num_rows($result);
        if ($first_record > $rows) return false; // invalid offset

        $offset = $first_record - 1;
        if (@pg_result_seek($result, $offset) !== true) {
            return false;
        }
        $row_count = 0;
        $return=array();
        while (($row_count < $number_of_records) AND ($row = db_fetch_array($result))) {
            $row_count ++;
            $return[]=$row;
        }
        return $return;
    }

    function db_close() {
        global $db_link;
        db_abort();
        if (!is_resource($db_link)) return false;
        return @pg_close($db_link);
    }

    function db_date($date) {
        if ($date === '') return 'NULL';
        if (is_array($date)) {
            if (array_key_exists('date',$date)) {
                $date = $date['date'];
            }
        }

        if (is_null($date)) return 'NULL';
        if ($date === '') return 'NULL';
        $dt = strtotime($date);
        return db_string(date('Y-m-d H:i:s O', $dt));
    }

    function db_quote($text) {
        return db_string($text);
    }

    function db_string_table($table_name) {
        return '"' . str_replace('"', '\"', $table_name) . '"';
    }

    function db_escape($text) {
        /* database must UTF8*/
        global $db_link;
        return pg_escape_string($db_link, utf8_encode($text));
    }
    function db_string($value, $type='varchar', $scale=0) {
        if (is_null($value)) return 'NULL';

        if ((is_array($value)) && (array_key_exists(0, $value))) return $value[0];

        switch ($type) {
            case 'varchar':
            case 'text':
                if ($value == '') return 'NULL';
                return "'" . db_escape($value) . "'";
                break;
            case 'date':
            case 'timestamptz':
            case 'timestamp':   return db_date($value);
            case 'bpchar':      return db_bit($value);
            case 'numeric':     return db_dec($value, $scale);

            case 'int8':
            case 'int4':        return db_int($value);
            default:
                die_err("Un-supported field-type: [{$type}] in db_string");
        }
    }

    function db_int($num) {
        if ($num === '') return 'NULL';
        if (is_array($num)) {
            if (array_key_exists('int', $num)) {
                $num = $num['int'];
            } elseif (array_key_exists('dec', $num)) {
                $num = $num['dec'];
            } elseif (array_key_exists(0, $num)) {
                $num = $num[0];
            }
        }

        if (is_null($num)) return 'NULL';
        if ($num === '') return 'NULL';

        return intval(val($num));
    }

    function db_dec($num, $scale) {
        if ($num === '') return 'NULL';
        if (is_array($num)) {
            if (isset($num['int'])) {
                $num = $num['int'];
            } elseif (isset($num['dec'])) {
                $num = $num['dec'];
            } elseif (isset($num[0])) {
                $num = $num[0];
            }
        }

        if (is_null($num)) return 'NULL';
        if ($num === '') return 'NULL';

        return number_format(val($num), $scale, $dec_pt='.', $th='');
    }

    function db_free($result) {
        if (!is_resource($result)) return false;
        return pg_free_result($result);
    }
    function db_bit($value) {
        if ((is_null($value)) || ($value === '')) return 'NULL';
        if ($value === '') return 'NULL';
        if (is_array($value)) {
            if (isset($value['bit'])) {
                $value = $value['bit'];
            } elseif (isset($value[0])) {
                $value = $value[0];
            }
        }

        return db_string((cbool($value) == true) ? 'T' : 'F');
    }


    function db_update($table, $fields, $where, $connection=null) {
        if (trim($table) == '') app_error('Missing table');
        if (!is_array($fields)) app_error('Missing fields');
        if (trim($where) == '') app_error('Missing where');

        $table_fields = db_table_def($table, $connection);


        $values=array();
        foreach ($fields as $field_name => $value) {
            /*
            if (is_array($value)) {
                foreach ($value as $key => $v) {
                    $value = $v;
                }
            }*/
            if (!isset($table_fields[$field_name])) {
                die_err("Invalid field: [{$field_name}] in table: [{$table}]", $fields);
            }
            if (is_null($value)) {
                $value = 'NULL';
            } else {
                $value = db_string($value, $table_fields[$field_name]['type'],$table_fields[$field_name]['scale']);
                if ($value === 'NULL') {
                    if ($table_fields[$field_name]['is_nullable'] != true) {
                        die_err("Error, field [{$field_name}] can not contain a null value, type=[{$table_fields[$field_name]['type']}]", $table_fields[$field_name]);
                    }
                }
            }
            $values[]=db_string_table($field_name) . " = " . $value;
        }

        $sql = "UPDATE  " . db_string_table($table) . "\n";
        $sql.= "SET     ";
        $sql .= implode(",\n        ", $values);
        $sql .= "\nWHERE   {$where}";
        $query = db_query($sql, $connection);
        $rows_affected=db_rows_affected($query, $connection);
        return $rows_affected;
    }


    function db_insert_uid($table, $fields, $id_field_name, $uid_field_name, $site_code_name='site_code', $connection=null) {
        global $config;
        if ((!isset($fields[$uid_field_name])) OR ($fields[$uid_field_name] == '')) {
            $fields[$uid_field_name] = date('YmdHis') . rand(10000, 99999);
        }
        if ((!isset($fields[$site_code_name])) OR ($fields[$site_code_name] == '')) {
            $fields[$site_code_name] = $config->site_code;
        }

        db_insert($table, $fields, $connection);
        $id = db_insert_id($connection);

        $uid = build_uid($fields[$site_code_name], $id);
        $where = "\"{$site_code_name}\" = '{$fields[$site_code_name]}' AND \"{$id_field_name}\" = $id";

        $fields=array(
            $uid_field_name => $uid
        );


        db_update($table, $fields, $where, $connection);

        return $uid;
    }

    function db_insert_seq($table, $fields, $connection=null) {
        db_insert($table, $fields, $connection);
        return db_insert_id($connection);
    }

    function db_insert($table, $fields, $connection=null) {

        if (trim($table) == '') app_error('Missing table');
        if (!is_array($fields)) app_error('Missing fields');


        $table_fields = db_table_def($table, $connection);

        $values=array();
        foreach ($fields as $field_name => $value) {
            $field_name = trim($field_name);
            /*
            if (is_array($value)) {
                foreach ($value as $key => $v) {
                    $value = $v;
                }
            }  */
            $value = db_string(
                $value,
                $table_fields[$field_name]['type'],
                $table_fields[$field_name]['scale']
            );
            if ($value === 'NULL') {
                if ($table_fields[$field_name]['is_nullable'] != true) {
                    die_err("Error, field [{$field_name}] can not contain a null value (" . print_r($table_fields[$field_name],1) . ")", $fields);
                }
            }
            $values[db_string_table($field_name)] = $value;
        }

        $insert_fields = '';
        foreach ($values as $field_name_escaped => $value_escaped) {
            $insert_fields .= (($insert_fields == '') ? '' : ', ') . $field_name_escaped;
        }
        $insert_values = implode(", ", $values);

        $sql = "INSERT\n";
        $sql .="INTO    " . db_string_table($table) . "\n";
        $sql .="        ({$insert_fields})\n";
        $sql .="VALUES  ({$insert_values})\n";
        
        $query = db_query($sql, $connection);
        return db_rows_affected($query, $connection);
    }



    function db_rows_affected($query_result, $connection=null) {
        global $db_link;
        $connection = ((is_null($connection)) ? $db_link : $connection);
        if (!is_resource($connection)) db_error($connection, 'db not connected or not a valid resource');
        if (!is_resource($query_result)) db_error($query_result, 'query_result is not a valid resource');
        $result = pg_affected_rows($query_result);
        return $result;
    }

    function db_num_rows($resource) {
        if (!is_resource($resource)) db_error(null, 'query passed is not a valid resource - run db_query first.');
        $result = @pg_num_rows($resource);
        return $result;
    }

    function db_query($sql, $connection = null) {
        global $db_link, $db_query_count, $db_query_time;

        $db_query_count ++;
        $start = _db_time();


        if (defined('DEBUG_SQL')) {
            echo '<hr><b>db_query()</b><pre>' . $sql . '</pre>';
        }

        $result = false;
        if (is_null($connection)) $connection = $db_link;

        if (!is_resource($connection)) db_error($connection, 'db not connected or not a valid resource');

        if (defined('DEBUG_SQL')) {
            // enable error reporting
            $result = @pg_query($connection, $sql);
            if (is_resource($result)) {
                echo '<b>Rows Returned:</b> ' . pg_num_rows($result) . '<br>';
                echo '<b>Rows Affected:</b> ' . pg_affected_rows($result) . '<br>';
                echo '<b>Time:</b>' . number_format(_db_time_diff($start), 2) . ' seconds<br>';

            }
        } else {
            $result = @pg_query($connection, $sql);
        }
        $db_query_time += _db_time_diff($start);

        if (is_resource($result)) {
            return $result;
        } elseif ($result === true) {
            return true; // ok
        } else {
            db_error($connection, "Query Failed to Run", $sql);
        }

    }
    function get_sql_error ($connection=null) {
        return 'no additional info';
    }

    function db_insert_id($_table_name=null, $_column_name=null, $_connection=null) {
        global $db_link;

        if (is_resource($_table_name)) {
            $connection = $_table_name;
            $table_name = $_column_name;
            $column_name = $_connection;
        } else {
            $connection = $_connection;
            $table_name = $_table_name;
            $column_name = $_column_name;
        }

        $connection = (is_null($connection) ? $db_link : $connection);

        if (!is_resource($connection)) db_error(null, 'db not connected or not a valid resource');

        if (is_null($table_name)) {
            $sql = "SELECT LASTVAL() as insert_id";
        } elseif (is_null($column_name)) {
            $sql = "SELECT CURRVAL(" . db_string($table_name) . ") as insert_id";
        } else {
            $sql = "SELECT pg_get_serial_sequence(" . db_string($table_name) . ", " . db_string($column_name) . ") as insert_id";
        }
        $q= db_query($sql, $connection);
        if (!db_num_rows($q)) {
            db_error($connection, 'Failed to find identity');
        }
        $r=db_fetch_array($q, 'assoc');
        return $r['insert_id'];
    }


    function db_fetch_array($resource, $type='assoc') {
        global $db_fetch_count, $db_fetch_time;
        if ($type === true) {
            db_error(null, 'true passed to db_fetch_array');
        }
        $db_fetch_count ++;
        $start = _db_time();


        //global $db_link;
        if (!is_resource($resource)) db_error(null, 'query passed is not a valid resource - run db_query first.');
        switch ($type) {
            case PGSQL_NUM: case 'num': case 'NUM':
                $result = pg_fetch_row($resource, null);
                break;
            case PGSQL_ASSOC: case 'assoc': case 'ASSOC':
                $result = pg_fetch_assoc($resource, null);
                break;
            case 'obj': case 'OBJ':
            	$result = pg_fetch_object($resource, null);
            	break;
            default:
                $result = pg_fetch_array($resource, null, PGSQL_BOTH);
                break;
        }

        $db_fetch_time += _db_time_diff($start);

        if ($result === false) { // still
            db_free($resource);
            return false;
        }
        return $result;

    }

    function db_get_fields($query, $connection = null) {
        global $db_link;

        $connection = (is_null($connection) ? $db_link : $connection);
        $query = db_query("SELECT * FROM ({$query}) DRVTBL LIMIT 0", $connection); // 0 records always returned.

        $field_count = pg_num_fields($query);
        $fields=array();
        for ($i=0;$i<$field_count;$i++) {
            $fields[]=new pg_field_class($query, $i);
        }
        db_free($query);
        return $fields;
    }

    function db_get_query_def($query, $connection=null) {
        $fields = db_get_fields($query, $connection);
        $return=array();
        foreach ($fields as $index => $f) {
            if ($f->name != 'xxxxrowguid') {
                switch ($f->type) {
                    case 'blob': $type='text'; break;
                    case 'real': $type='decimal'; break;
                    case 'char': $type='varchar'; break;
                    default:
                        $type=$f->type;
                }

                $return[$f->name]=array(
                    'name'=>$f->name,
                    'short_name'=>substr($f->name, 0, 30),
                    'type'=>$type,
                    'length'=>$f->max_length,
                    'decimal_places'=>2
                );
            }
        }
        return $return;
    }



    function db_table_def($table_name, $key_field_name=false, $connection=null) { // IC2007 specific function
        static $cache=array();
        if (isset($cache[$table_name])) return $cache[$table_name];
        $sql = "SELECT  c.relname as table_name,
                        a.attnum as column_order,
                        a.attname AS column_name,
                        t.typname AS column_type,
                        a.attlen AS length,
                        CASE t.typname
                            WHEN 'bpchar' THEN a.atttypmod - 4
                            WHEN 'varchar' THEN a.atttypmod - 4
                            WHEN 'numeric' THEN (a.atttypmod - 4 ) / 65536
                            WHEN 'decimal' THEN (a.atttypmod - 4 ) / 65536
                            ELSE NULL
                        END  AS scale,
                        CASE t.typname
                            WHEN 'numeric' THEN (a.atttypmod - 4 ) % 65536
                            WHEN 'decimal' THEN (a.atttypmod - 4 ) % 65536
                            ELSE NULL
                        END AS prec,
                        a.atttypmod AS lengthvar,
                        CASE WHEN a.attnotnull = 't' THEN 'F' ELSE 'T' END as isnullable,
                        d.adsrc as default_text,
                        CASE WHEN d.adsrc ILIKE 'nextval(%' THEN 'T' ELSE 'F' END as isautonumber,
                        'F' as iscomputed,
                        t.typname as xtype
                FROM    pg_attribute a
                        INNER JOIN pg_class c ON c.oid = a.attrelid
                        INNER JOIN pg_type t ON t.oid = a.atttypid
                        LEFT OUTER JOIN pg_attrdef d ON c.oid = d.adrelid AND a.attnum = d.adnum
                WHERE   a.attnum > 0
                        AND c.relname = '{$table_name}'
                ORDER BY a.attnum";

        $query = db_query($sql, $connection);
        if (!db_num_rows($query)) {
            db_error($connection, 'Invalid table: ' . $table_name);
        }

        $return=array();
        while ($row = db_fetch_array($query)) {
            if (strpos($row['default_text'], "nextval('") !== false) $row['default_text'] = '';
            $row['caption'] = ucwords(str_replace('_', ' ', $row['column_name']));
            if ($row['length'] < 0) {
                if ($row['scale'] > 0) {
                    $row['length'] = $row['scale'];
                } else {
                    $row['length'] = 999999999;
                }
            }

            $default_value = trim($row['default_text']);
            if ($default_value != '') {
                $default = db_fetch_array(db_query("SELECT  {$default_value} as def"), 'assoc');
                $default_value = $default['def'];
            }


            if ($row['column_name'] != 'xxxxrowguid') {
                $field=array(
                    'short_name'    => $row['column_name'], // mssql hangover.
                    'name'          => $row['column_name'],
                    'type'          => $row['column_type'],
                    'default'       => $default_value,
                    'is_nullable'   => ($row['isnullable']=='T' ? true : false),
                    'length'        => $row['length'],
                    'scale'         => $row['scale'],
                    'decimal_places'=> $row['prec'],
                    'is_computed'   => ($row['iscomputed']=='T' ? true : false),
                    'is_identity'   => ($row['isautonumber']=='T' ? true : false),
                    'caption'       => $row['caption']
                );
            }

            $return[$field['name']] = $field;
        }
        $cache[$table_name] = $return;
        return $return;
    }

    function db_load_table_row($sql, $connection=null) {
        global $db_fetch_time, $db_fetch_count;
        $query=db_query($sql, $connection);
        if (db_num_rows($query) != 1) {
            db_error($connection, 'Error, db_load_table_row() resulted in ' . db_num_rows($query) . ' record(s). <br>Expecting 1 record.', $sql);
        }

        $start_time = microtime(true);
        $db_fetch_count++;
        $return = db_fetch_array($query, 'assoc');
        db_free($query);
        $db_fetch_time += (microtime(true)-$start_time);
        return $return;
    }

    function db_get_table_def($table_name, $key_field_name=false, $connection=null) {
        global $db_misc_time;
        static $cache=array();
        if (isset($cache[$table_name][$key_field_name])) return $cache[$table_name][$key_field_name];
        $start_time = microtime(true);
        $sql = "SELECT  c.relname as table_name,
                        a.attnum as column_order,
                        a.attname AS column_name,
                        t.typname AS column_type,
                        a.attlen AS length,
                        CASE t.typname
                            WHEN 'bpchar' THEN a.atttypmod - 4
                            WHEN 'varchar' THEN a.atttypmod - 4
                            WHEN 'numeric' THEN (a.atttypmod - 4 ) / 65536
                            WHEN 'decimal' THEN (a.atttypmod - 4 ) / 65536
                            ELSE NULL
                        END  AS scale,
                        CASE t.typname
                            WHEN 'numeric' THEN (a.atttypmod - 4 ) % 65536
                            WHEN 'decimal' THEN (a.atttypmod - 4 ) % 65536
                            ELSE NULL
                        END AS prec,
                        a.atttypmod AS lengthvar,
                        CASE WHEN a.attnotnull = 't' THEN 'F' ELSE 'T' END as isnullable,
                        d.adsrc as default_text,
                        CASE WHEN d.adsrc ILIKE 'nextval(%' THEN 'T' ELSE 'F' END as isautonumber,
                        'F' as iscomputed,
                        t.typname as xtype
                FROM    pg_attribute a
                        INNER JOIN pg_class c ON c.oid = a.attrelid
                        INNER JOIN pg_type t ON t.oid = a.atttypid
                        LEFT OUTER JOIN pg_attrdef d ON c.oid = d.adrelid AND a.attnum = d.adnum
                WHERE   a.attnum > 0
                        AND c.relname = '{$table_name}'
                ORDER BY a.attnum";

        $query = db_query($sql, $connection);
        if (!db_num_rows($query)) {
            db_error($connection, 'Invalid table: ' . $table_name);
        }

        $return=array();
        while ($row = db_fetch_array($query)) {
            if (strpos($row['default_text'], "nextval('") !== false) $row['default_text'] = '';
            $row['caption'] = ucwords(str_replace('_', ' ', $row['column_name']));
            if ($row['length'] < 0) $row['length'] = 999999999;
            $default_value = $row['default_text'];
            if ($default_value != '') {
                $default = db_fetch_array(db_query("SELECT  {$default_value} as def"), 'assoc');
                $default_value = $default['def'];
            }

            if ($row['column_name'] != 'xxxxrowguid') {
                $field=array(
                    'short_name'=>$row['column_name'], // was 30char for mssql
                    'name'=>$row['column_name'],
                    'type'=>$row['column_type'],
                    'default'=>$default_value,
                    'is_nullable'=>($row['isnullable']=='T' ? true : false),
                    'length'=>$row['length'],
                    'scale'=>$row['scale'],
                    'decimal_places'=>$row['prec'],
                    'is_computed'=>($row['iscomputed']=='T' ? true : false),
                    'is_identity'=>($row['isautonumber']=='T' ? true : false),
                    'caption'=>$row['caption']
                );
            }
            if ($key_field_name == true) {
                $return[$field['name']] = $field;
            } else {
                $return[]=$field;
            }
        }
        $cache[$table_name][$key_field_name] = $return;
        //echo '<br>DB Update ';
        //var_dump($return);
        $db_misc_time += (microtime(true)-$start_time);
        return $return;
    }


    function db_insert_table_from_array($table_name, $fields, $array) {
        $x = new Object();
        foreach ($array as $key => $field) {
            $x->$key = $field;
        }
        return db_insert_table($table_name, $fields, $x);
    }

    function db_insert_table($table_name, $fields, &$table_object, $connection=null) {
        // create basic update statement
        $counter=0;
        $insert_fields = $insert_values = '';
        foreach ($fields as $index => $field) {
            if ($field['type'] != 'uniqueidentifier') {     // leave guid alone
                if ($field['is_computed'] == false) {       // leave calculated fields alone
                    if ($field['is_identity'] == false) {   // leave identity fields alone
                        $counter++;
                        if ($counter > 1)  {
                             $insert_fields .= ', ';
                             $insert_values .= ', ';
                        }
                        $insert_fields .= '"' . $field['name'] . '"';
                        $insert_values .= db_format_field($field, $table_object->$field['name']);
                    }
                }
            }
        }
        $sql = "INSERT INTO \"$table_name\" ({$insert_fields}) VALUES ({$insert_values}) ";
        db_query($sql, $connection);
        return true;
    }
    function db_format_field($field, $value) {
        $value = trim($value);
        if ((is_array($value)) && (isset($value[0]))) {
            return $value[0];
        }
        if (is_null($value)) return 'NULL';
        if ($value === '') return 'NULL';

        switch ($field['type']) {
            case 'bpchar':
            case 'bit':
                switch (trim(strtolower($value))) {
                    case 'y':
                    case 'yes':
                    case 'true':
                    case -1;
                    case 1;
                    case '1':
                    case '-1':
                    case 't':
                        return db_string('T'); // true
                        break;
                }
                if ($value === true) {
                    return db_string('T');
                }
                if ($value === '') {
                    if ($field['is_nullable'] == true) {
                        return 'NULL';
                    }
                }
                return db_string('F'); // false;
                break;

            case 'varchar':
            case 'char':
                if ($value === '') {
                    if ($field['is_nullable'] == true) {
                        return 'NULL';
                    }
                }
                return db_string(substr($value,0,$field['length']));
                break;

            case 'int8':
            case 'int4':
            case 'int':
            case 'bigint':      // integer
            case 'smallint':
                if ($value === '') {
                    if ($field['is_nullable'] == true) {
                        return 'NULL';
                    }
                }
                $value = intval(val($value));   // convert to integer
                return $value;
                break;

            case 'float':
            case 'real':
            case 'decimal':     // scale specific
            case 'money':
            case 'numeric':
                if ($value === '') {
                    if ($field['is_nullable'] == true) {
                        return 'NULL';
                    }
                }
                $dp = val($field['decimal_places']);
                $value = val($value);
                return number_format($value, $dp, $dec_sep='.', $tho_sep='');
                break;

            case 'timestamptz':
            case 'timestamp':
            case 'datetime':
                if ($value == '') {
                    if ($field['is_nullable']==false) {
                        return "'" . date('Y-m-d H:i:s') . "'";
                    } else {
                        return 'NULL';
                    }
                }
                if (substr(strtolower($value),0,4) == 'date') {
                    return $value; // dateadd, datepart
                }
                if ((strtolower($value) == 'getdate()') OR (strtolower($value) == 'now()')) {
                    return "'" . date('Y-m-d H:i:s') . "'";
                }
                //$dt = new dateclass($value);
                //return $dt->db_format();
                if ($value == '') {
                    return 'NULL';
                } else {
                    return db_string(date('d M Y H:i:s', strtotime($value)));
                }
                break;



            case 'smalldatetime':
                db_error(null, 'smalldatetime not supported by db->format_field()');
                break;



            case 'text':        // use write_text to save text fields.
                if ($value === '') {
                    if ($field['is_nullable'] == true) {
                        return 'NULL';
                    }
                }
                return db_string($value);
                break;

            case 'bytea':
                return 'NULL';
                break;

            default:
                die_err("un-supported field type: [{$field['type']}]"); //, $field);
                break;

        }

    }
    function db_update_table_from_array($table_name, $fields, $array, $where) {
        $x = new Object();
        foreach ($array as $key => $field) {
            $x->$key = $field;
        }
        return db_update_table($table_name, $fields, $x, $where);
    }

    function db_update_table($table_name, $fields, &$table_object, $where,$connection=null) {
            // create basic update statement
            $table_name = str_replace(array('[', ']'), '', $table_name);
            $sql = "UPDATE \"{$table_name}\" SET ";
            $counter=0;
            foreach ($fields as $field) {
                if (isset($table_object->$field['name'])) {
                    if ($field['type'] != 'uniqueidentifier') {     // leave guid alone
                        if ($field['is_computed'] == false) {       // leave calculated fields alone
                            if ($field['is_identity'] == false) {   // leave identity fields alone
                                $counter++;
                                if ($counter > 1)  {
                                    $sql .= ', ';
                                }
                                $sql .= '"' . $field['name'] . '" = ' . db_format_field($field, $table_object->$field['name']);
                            }
                        }
                    }
                } else {
                    db_error($connection, "table field not set: {$field['name']}", $table_object);
                }
            }

            $sql .= $where;
            db_query($sql, $connection);

            return true;
        }

    function db_lookup($sql, $connection=null) {
        $result = db_fetch_array(db_query($sql,$connection), 'num');
        if (!isset($result[0])) db_error($connection, 'Lookup failed, no return record', $sql);
        return $result[0];
    }

