<?php
if ($_GET["data"]){
	file_put_contents('data.txt', $_GET["data"], FILE_APPEND);
}

echo json_encode(array('time' => date("d/m/Y H:i", time())));

?>
