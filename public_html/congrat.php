<?php
session_start();
require_once("include/config.php");
require_once("include/HttpClient.class.php");

?>

<html>

<head>
<title>Congratulations! - Ptwitter</title>
</head>

<body>

<div style="float: right">
<?php
if(isset($_SESSION['username'])) {
	echo "<font color='white' size='4'><b>$_SESSION[username]</b></font>";
}
else {
	echo "<meta http-equiv='REFRESH' content='0;url=index.php'>";
}
?>
</div>

<div style="background: blue">
<h1><font color="white">PTWITTER</font></h1>
</div>

<div>
	<hr/>
	<div align="left">
		<a href="home.php" target="_self"> <font color="blue" size="4">Home</font> </a> &nbsp &nbsp
		<a href="follow.php" target="_self"> <font color="blue" size="4">Following & Followers</font> </a> &nbsp &nbsp
		<a href="index.php?signout" target="_self"> <font color="blue" size="4">Sign Out</font> </a>
	</div>
	<hr/>
</div>

<table border="1" width="800px" align="center">
	<tr> <td>
	<center><h3>Congartulations! You have successfully registered for Ptwitter</h3></center>
	</td> </tr>
</table>

</body>

</html>	
	