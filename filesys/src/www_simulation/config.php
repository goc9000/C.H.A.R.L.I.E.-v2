<?php
	require('_test_common.php');
	
	page_header();
	
	$tab = $_REQUEST['t'] ?: 1;
	$cmd = $_REQUEST['C'] ?: 0;
	$exe = $cmd;
?>
<page id="configPage" executed="<?php echo $exe; ?>">
	<config tab="<?php echo $tab; ?>">
		<plants>
			<plant min="25" max="75" flags="0" />
			<plant min="25" max="75" flags="0" />
			<plant min="25" max="75" flags="1" />
			<plant min="25" max="75" flags="0" />
		</plants>
		<recording interval="3600" />
		<net mac="12:34:56:78:90:AB" />
		<alerts ip="10.1.1.1" port="10000" />
		<timesvr ip="12.34.56.78" tzdelta="3" />
	</config>
</page>
