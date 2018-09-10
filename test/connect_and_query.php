<?php
$server = 'localhost:30515';
//$dbName = 'databaseName';
$uid = 'SYSTEM';
$pwd = 'manager';

$options = array("UID"=>$uid, "PWD"=>$pwd);
$conn = hdb_connect($server, $options);
if($conn === false) {
    die(print_r(hdb_errors(), true));

}

$tableName = 'PHP_TEST';
$query = "drop TABLE $tableName;";
$stmt = hdb_query($conn, $query);
if($stmt === false) {
    //die(print_r(hdb_errors(), true));

}
$query = "CREATE TABLE $tableName (A int, B NVARCHAR(20))";

$stmt = hdb_query($conn, $query);
if($stmt === false) {
    die(print_r(hdb_errors(), true));

}
hdb_free_stmt($stmt);

//$query = "INSERT INTO $tableName (A, B) VALUES (2018, 'dddd')";
//$stmt = hdb_query($conn, $query);
//if($stmt === false) {
//    die(print_r(hdb_errors(), true));
//
//}
//$query = "SELECT * FROM $tableName";
//$stmt = hdb_query($conn, $query);
//if(hdb_fetch($stmt) === false) {
//    die(print_r(hdb_errors(), true));
//
//}
//$col1 = hdb_get_field($stmt, 0);
//echo "First field:  $col1 \n";
//
//$col2 = hdb_get_field($stmt, 1);
//echo "Second field:  $col2 \n";

//hdb_free_stmt($stmt);


for ($i = 0 ; $i < 5; $i++) {

    $query = "INSERT INTO $tableName (A, B) VALUES (1, '中国')";
    $stmt = hdb_query($conn, $query);
    if($stmt === false) {
        die(print_r(hdb_errors(), true));
    
    }
    hdb_free_stmt($stmt);
}
    $query = "INSERT INTO $tableName (A, B) VALUES (1, '中国从此站起来了')";
    $stmt = hdb_query($conn, $query);
    if($stmt === false) {
        die(print_r(hdb_errors(), true));
    
    }
hdb_free_stmt($stmt);

$query = "SELECT * FROM $tableName";
$stmt = hdb_query($conn, $query);

if(hdb_fetch($stmt) === false) {
    die(print_r(hdb_errors(), true));

}

$fieldNum = hdb_num_fields($stmt);
echo "field number $fieldNum \n";

$col1 = hdb_get_field($stmt, 0);
echo "First field:  $col1 \n";

$col2 = hdb_get_field($stmt, 1);
echo "Second field:  $col2 \n";

hdb_free_stmt($stmt);

//$query = "UPDATE $tableName SET B='中国人' WHERE B='中国从此站起来了'";
$query = "UPDATE $tableName SET B=(?) WHERE B=(?)";
$params = array("中国人","中国从此站起来了");
$stmt = hdb_prepare($conn, $query, $params);
if( $stmt )  
{  
     echo "Statement prepared.\n";  
}  
else  
{  
     echo "Error in preparing statement.\n";  
     die( print_r( hdb_errors(), true));  
}  
if( hdb_execute( $stmt))  
{  
      echo "Statement executed.\n";  
}
else  
{  
     echo "Error in executing statement.\n";  
     die( print_r( hdb_errors(), true));  
}   

hdb_free_stmt($stmt);

$query = "SELECT * FROM $tableName";
$stmt = hdb_query($conn, $query);
if($stmt === false) {
    die(print_r(hdb_errors(), true));

}
while($row =hdb_fetch_array($stmt)){
echo $row['A'].", ".$row['B']."\n";
}


hdb_free_stmt($stmt);
hdb_close($conn);

?>
