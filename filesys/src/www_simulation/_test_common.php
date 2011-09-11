<?php
	define('DATE_FMT', 'Y-m-d\TH:i:s');

	function page_header()
	{
		header('Content-type: text/xml');
		echo '<'.'?xml version="1.0" encoding="UTF-8" ?'.'>';
		echo '<'.'?xml-stylesheet type="text/xsl" href="main.xsl" ?'.'>';
	}
	
	function fmt_time($time) {
		return $time ? date(DATE_FMT, $time) : '0000-00-00T00:00:00';
	}
	
	function print_event($event)
	{
		echo '<event datetime="' . fmt_time($event['time'])
			. '" code="' . $event['code']
			. '" data="' . $event['data'] . '" />';
	}
	
	function print_plant($plant)
	{
		echo '<plant';
		if (isset($plant['humid'])) {
			echo ' humid="' . $plant['humid'] . '"';
			echo ' ilum="' . $plant['ilum'] . '"';
		}
		echo '/>';
	}
	
	function print_record($record)
	{
		echo '<row datetime="' . fmt_time($record['time']) . '">';
		foreach ($record['plants'] as $plant) {
			print_plant($plant);
		}
		echo '</row>';
	}
	
	function gen_log($from_date, $to_date)
	{
		global $LOG;
		
		$LOG = array();
		$CODES = array(1,2,3,65,66,129);
		
		for ($time = $from_date; $time < $to_date; $time += rand(30, 3600) << rand(1,4)) {
			$code = $CODES[rand(0, count($CODES)-1)];
			$data = rand(1,4);
			$LOG[] = array(
				'time' => $time,
				'code' => $code,
				'data' => $data
			);
		}
	}
	
	function gen_records($from_date, $to_date)
	{
		global $RECORDS;
		
		$RECORDS = array();
		
		for ($time = $from_date; $time < $to_date; $time += 3600) {
			$plants = array();
			for ($i = 1; $i <= 4; $i++) {
				$plants[] = ($i == 3) ? array() : array(
					'humid' => rand(0,100),
					'ilum' => rand(0,100)
				);
			}
			
			$RECORDS[] = array(
				'time' => $time,
				'plants' => $plants
			);
		}
	}
	
	function read_pagination_params() {
		$y1 = $_REQUEST['Y'] ?: 0;
		$m1 = $_REQUEST['M'] ?: 0;
		$d1 = $_REQUEST['D'] ?: 0;
		$y2 = $_REQUEST['y'] ?: 0;
		$m2 = $_REQUEST['m'] ?: 0;
		$d2 = $_REQUEST['d'] ?: 0;
		
		$date_from = ($y1 && $m1 && $d1) ? mktime(0,0,0,$m1,$d1,$y1) : 0;
		$date_to = ($y2 && $m2 && $d2) ? mktime(0,0,0,$m2,$d2,$y2) : 0;
		
		$params = array(
			'date_from' => $date_from,
			'date_to' => $date_to,
			'per_page' => (int)($_REQUEST['n'] ?: 25),
			'page' => (int)($_REQUEST['p'] ?: 1),
			'reverse' => (int)($_REQUEST['r'] ?: 0)
		);
		
		return $params;
	}
	
	function apply_pagination($params, &$data) {
		$result = array_filter($data, function($entry) use ($params) {
			if ($params['date_from'] && ($entry['time'] < $params['date_from'])) {
				return false;
			}
			if ($params['date_to'] && ($entry['time'] >= ($params['date_to']+86400))) {
				return false;
			}
			
			return true;
		});
		
		if ($params['reverse']) {
			$result = array_reverse($result);
		}
		
		$params['pages'] = floor((count($result) + $params['per_page'] - 1) / $params['per_page']);
		$params['page'] = min($params['pages'], max(1, $params['page']));
		
		return array(
			$params,
			array_slice($result, ($params['page']-1) * $params['per_page'], $params['per_page'])
		);
	}
	
	function write_pagination_params($params) {
		echo 'from="' . fmt_time($params['date_from']) . '" ' .
			'to="' . fmt_time($params['date_to']) . '" ' .
			'perPage="' . $params['per_page'] . '" ' .
			'pages="' . $params['pages'] . '" ' .
			'page="' . $params['page'] . '" ' .
			'reverse="' . $params['reverse'] . '" ';
	}
	
	srand(123456);
	gen_log(mktime(0,0,0, 04, 15, 2010), time());
	gen_records(mktime(0,0,0, 04, 15, 2010), time());
