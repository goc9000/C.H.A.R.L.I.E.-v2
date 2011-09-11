<?php
	require('_test_common.php');
	
	page_header();
	
	$params = read_pagination_params();
	list($params,$data) = apply_pagination($params,$RECORDS);
?>
<page id="recordsPage" datetime="<?php echo date(DATE_FMT); ?>">
	<records <?php write_pagination_params($params); ?>>
	<?php
		foreach ($data as $entry) {
			print_record($entry);
		}
	?>
	</records>
</page>
