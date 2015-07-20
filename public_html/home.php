<?php
session_start();
require_once("include/config.php");
require_once("include/HttpClient.class.php");
?>

<html>

<head>
<title>Homepage - Ptwitter</title>
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
	<h3>What's happening?</h3>
	
	<form method="post" action="home.php"> 
		<textarea rows="4" cols="50" name="status"></textarea><br/>
		<input type="submit" value="Update">
	</form>
</div>

<?php
//------------------------- UPDATE TWEET -----------------------//

if(isset($_POST['status'])) {
	$status = $_POST['status'];
	
	$client->setAuthorization($_SESSION['username'], $_SESSION['pass']);
	$client->post('/statuses/update.xml', array(
    'status' => $status));
	
	$xmlReply1 = simplexml_load_string($client->getContent());
	
	if($client->getStatus() != '200') {
		echo "<center> <font color='red'>$xmlReply1->error</font> </center>";
	}
	else {
		echo "<center> <h3>Update successfully!</h3> </center>";
	}
}
?>

</br>


<div align="center">
	</br>
	<h3>View Recent Tweets</h3>
	<form method="post" action="home.php">
		Username:	<input type="text" size="15" maxlength="25" name="screen_name">
		Number of tweets:	<input type="text" size="10" maxlength="25" name="count"> </br>
		<input type="submit" value="View Tweets">
	</form>	
</div>	

<?php
//-------------------------- VIEW RECENT TWEETS --------------------------//

if(isset($_POST['screen_name'])) {
	echo "<table align='center' width='800px' border='1'><tr><td>";
	echo "<center><h3>$_POST[screen_name]'s Latest Tweets</h3></center>";
	
	$client->setAuthorization($_SESSION['username'], $_SESSION['pass']);
	$request = "/statuses/user_timeline/";
	$request .= $_POST['screen_name'];
	$request .= ".xml";
	
	$client->get($request, array(
	'count' => $_POST['count'] ));
	
	$xmlReply = simplexml_load_string($client->getContent());
	
	if($client->getStatus() != '200') {
		echo "<tr><td><center> <font color='red'>$xmlReply->error</font> </center></td></tr>";
	}
	else if(empty($xmlReply->status)) {
		echo "<tr><td>";
		echo "<center><h4>This user has no tweet.</h4></center>";
		echo "</td></tr>";
	}
	else {
		foreach ($xmlReply->status as $value) {
			$username = $value->user->screen_name;
			$text = urldecode($value->text);
			echo "<tr><td>";
			echo "<font size='3' color='blue'><b>$username</b></font>"." - "."<font size='2'>$value->created_at</font><br/>";
			echo "<font size='4'>$text</font>";
			echo "</td></tr>";
		}
	}
	echo "</td></tr></table>";	
}

?>

</body>

</html>
