<?php
$testMode = $_GET["testing"]?:0;
$data = "";

if($testMode)
{
	error_reporting(E_ALL);
	ini_set('display_errors', 'on');
}

$data =  file_get_contents("php://input");
if($data == "")
{
	if($testMode)
	{
		header('Content-Type: application/json');
		$data = file_get_contents("test.blux");
	}
}

$json = json_decode($data);
$appVersion = $testMode?$_GET["appVersion"]:$json->appVersion;
$fileVersion = $json->metaData->version;

// PROCESS


echo json_encode($json,JSON_UNESCAPED_SLASHES);



//HELPERS FUNCTIONS

function isUpdatingTo($targetVersion)
{
	global $appVersion, $fileVersion;
	return version_compare($appVersion, $targetVersion) >= 0 && version_compare($fileVersion, $targetVersion) < 0;
}

function getValueForParam($object, $name)
{
	foreach($object->parameters as $fKey => $param)
	{
		if($param->controlAddress == "/".$name) return $param->value;
	}

	return "notfound";
}


function setValueForParam(&$object, $name, $value)
{
	foreach($object->parameters as $fKey => $param)
	{
		if($param->controlAddress == "/".$name)
		{
			$param->value = $value;
		}
	}
}


?>
