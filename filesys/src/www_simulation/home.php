<?php
	require('_test_common.php');
	
	page_header();
?>
<page id="homePage" datetime="1985-09-30T04:11:51">
	<current>
		<?php
			$latest = $RECORDS[count($RECORDS)-1];
			foreach ($latest['plants'] as $entry) {
				print_plant($entry);
			}
		?>
	</current>
	<log>
		<?php
			foreach (array_reverse(array_slice($LOG, count($LOG)-5, 5)) as $entry) {
				print_event($entry);
			}
		?>
	</log>
	<records>
		<?php
			foreach (array_reverse(array_slice($RECORDS, count($RECORDS)-5, 5)) as $entry) {
				print_record($entry);
			}
		?>
	</records>
</page>
