<?php

if(strcmp($_POST["pass"],"rmpupload") == 0)
{
	if (move_uploaded_file($_FILES['appFile']['tmp_name'], "../download/app/".basename($_FILES['appFile']['name']))) {
	    echo "File is valid, and was successfully uploaded.\n";
	} else {
	    echo "Possible file upload attack!\n";
	}
}else
{
	echo "Pass is invalid or error retrieving data : ".$_POST["pass"]."\n";
}

?>