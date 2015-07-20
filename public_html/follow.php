<?php
session_start();
require_once("include/config.php");
require_once("include/HttpClient.class.php");
?>

<html>

<head>
<title>Followers - Ptwitter</title>
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

<div align="center">
	<h3>Follow A Friend</h3>
	<form method="post" action="follow.php">
		Username:	<input type="text" size="15" maxlength="25" name="following"> </br>
		<input type="submit" value="Start Follow">
	</form>	
</div>	

<?php
//------------------------- FOLLOW A FRIEND --------------------//
if(isset($_POST['following'])) {
	$client->setAuthorization($_SESSION['username'], $_SESSION['pass']);
	$request = "/friendships/create/";
	$request .= $_POST['following'];
	$request .= ".xml";
	
 	$client->post($request, $data);
	$xmlReply = simplexml_load_string($client->getContent());
	
	if($client->getStatus() != '200') {
		echo "<center> <font color='red'>$xmlReply->error</font> </center>";
	}
	else {
		echo "<center> <h3>You have successfully followed this user!</h3> </center>";
	}
}	
?>

</br>

<table border="1" width="400px" align="center">
	<tr><td><center><h3>Your Followers</h3></center></td></tr>
	
	<?php
	//---------------------- VIEW FOLLOWERs -----------------------//
	
	if(isset($_POST['nextpage'])) { //if user choo next page
		//send request
		$client->setAuthorization($_SESSION['username'], $_SESSION['pass']);
		$client->get('/statuses/followers.xml', array(
		'cursor' => $_POST['next'] ));

		//get reply
		$xmlReply = simplexml_load_string($client->getContent());
	
		foreach ($xmlReply->users->user as $value) {
			echo "<tr><td><center>";
			echo $value->screen_name;
			echo "</center></td></tr>";
		}

		$next = $xmlReply->next_cursor;
		$previous = $xmlReply->previous_cursor;
	}
	else if(isset($_POST['previouspage'])) { //if user choose previous page
		//send request
		$client->setAuthorization($_SESSION['username'], $_SESSION['pass']);
		$client->get('/statuses/followers.xml', array(
		'cursor' => $_POST['previous'] ));

		//get reply
		$xmlReply = simplexml_load_string($client->getContent());
	
		foreach ($xmlReply->users->user as $value) {
			echo "<tr><td><center>";
			echo $value->screen_name;
			echo "</center></td></tr>";
		}	

		$next = $xmlReply->next_cursor;
		$previous = $xmlReply->previous_cursor;
	}
	else if(isset($_POST['followers'])) { //the first time user view followers
		//send request
		$client->setAuthorization($_SESSION['username'], $_SESSION['pass']);
		$client->get('/statuses/followers.xml', array(
		'cursor' => -1 ));

		//get reply
		$xmlReply = simplexml_load_string($client->getContent());

		if($client->getStatus() != '200') { //error message
			echo "<center> <font color='red'>$xmlReply->error</font> </center>";
		}
		else if(empty($xmlReply->users)) { //no follower
			echo "<tr><td>";
			echo "<center><h4>You have no follower.</h4></center>";
			echo "</td></tr>";
		}
		else { //there are follower
			foreach ($xmlReply->users->user as $value) {
				echo "<tr><td><center>";
				echo $value->screen_name;
				echo "</center></td></tr>";
			}
		}
		$next = $xmlReply->next_cursor;
		$previous = $xmlReply->previous_cursor;
	}
	else
	{
		echo "<tr><td valign='center'>";
		echo "<center><form method='post' action='follow.php'>
					<input type='submit' value='View Followers' name='followers'>
					</form></center>";	
		echo "</td></tr>";
	}
	?>
	
</table>

<table border="0" width="400px" align="center">	
	<tr>
	<?php
	if($previous > 0) {
		echo "<td align='center'>";
		echo "<form method='post' action='follow.php'> 
					<input type='hidden' name='previous' value='$previous'> 
					<input type='submit' value='Previous' name='previouspage'> </form>";
		echo "</td>";
	}
	if($next > 0) {
		echo "<td align='center'>";
		echo "<form method='post' action='follow.php'> 
					<input type='hidden' name='next' value='$next'> 
					<input type='submit' value='Next' name='nextpage'> </form>";
		echo "</td>";
	}	
	?>
	</tr>
</table>	


</body>

</html>