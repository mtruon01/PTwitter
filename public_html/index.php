<?php
session_start();
require_once("include/config.php");
require_once("include/HttpClient.class.php");

if(isset($_GET['signout'])) {
	session_unset();
	session_destroy();
}

?>

<html>

<head>
<title>Ptwitter</title>
</head>

<body>
<div style="background: blue">
<h1><font color="white">PTWITTER</font></h1>
</div>

<?php
//------------------------ LOG IN -------------------------//

if(isset($_POST['username'])) {
	$username=$_POST['username'];
	$pass=$_POST['pass'];
	
	//send request
	$client->setAuthorization($username, $pass);
	$client->post('/login.xml', array(
    'screen_name' => $username,
    'password' => $pass ));
	
	//get reply
	$xmlReply = simplexml_load_string($client->getContent());
	
	//display reply
	if($client->getStatus() != '200') {
		echo "<center> <font color='red'>$xmlReply->error</font> </center>";
	}
	else {
		$_SESSION['username'] = $username;
		$_SESSION['pass'] = $pass;
		echo "<meta http-equiv='REFRESH' content='0;url=home.php'>";
	}
}

//------------------------------- REGISTER -----------------------------//

if(isset($_POST['screen_name']) AND isset($_POST['password'])) {
	$screen_name=$_POST['screen_name'];
	$password=$_POST['password'];
		
	//send request
	$client->post('/account/register.xml', array(
    'screen_name' => $screen_name,
    'password' => $password ));
	
	//get reply
	$xmlReply = simplexml_load_string($client->getContent());
	
	//display reply
	if($client->getStatus() != '200') {
		echo "<center> <font color='red'>$xmlReply->error</font> </center>";
	}
	else {
		$_SESSION['username'] = $screen_name;
		$_SESSION['pass'] = $password;
		echo "<meta http-equiv='REFRESH' content='0;url=congrat.php'>";
	}
}

?>

<center>
<table border="0" width="800px">
<tr>

	<td> 
		<center>
		<form method="post" action="index.php"> 
		<table border="0">

		<tr align="center"> <h3>Sign In<h3> </tr>

		<tr>
			<td valign="center" align="right"> Username: </td>

			<td valign="center" align="left"> <input type="text" size="15" maxlength="25" name="username"> </td>
		</tr>

		<tr>
			<td valign="center" align="right"> Password: </td>
			<td valign="center" align="left"> <input type="password" size="15" maxlength="25" name="pass"> </td>

		</tr>

		<tr> <td align="center" colspan="2"> <input type="submit" value="Sign in"> </td> </tr>

		</table> </form>
		</center> 
	</td>

	<td> 
		<center>
		<form method="post" action="index.php"> 
		<table border="0">

		<tr align="center"> <h3>Sign Up</h3> </tr>

		<tr>
			<td valign="center" align="right"> Username: </td>

			<td valign="center" align="left"> <input type="text" size="15" maxlength="25" name="screen_name"> </td>   
		</tr>

		<tr>
			<td valign="center" align="right"> Password: </td>
			<td valign="center" align="left"> <input type="password" size="15" maxlength="25" name="password"> </td>
		</tr>

		<tr> <td colspan="2" align="center"> <input type="submit" value="Register"> </td> </tr>

		</table> </form> 
		</center>
	</td>

</tr>
</table>
</center>

</body>

</html>

