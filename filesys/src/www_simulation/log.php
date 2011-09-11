<?php
	require('_test_common.php');
	
	page_header();
	
	$params = read_pagination_params();
	list($params,$data) = apply_pagination($params,$LOG);
?>
<page id="logPage" datetime="<?php echo date(DATE_FMT); ?>">
	<log <?php write_pagination_params($params); ?>>
	<?php
		foreach ($data as $entry) {
			print_event($entry);
		}
	?>
	</log>
</page>
