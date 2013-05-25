<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="2.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:output method="xml" 
		media-type="text/html" 
		doctype-public="-//W3C//DTD XHTML 1.0 Transitional//EN"
		doctype-system="http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd"
		cdata-section-elements="script style"
		indent="yes"
		encoding="UTF-8"/>
	<xsl:template match="/page">
		<html xmlns="http://www.w3.org/1999/xhtml">
		<head>
			<title>C.H.A.R.L.I.E. v2</title>
			<link rel="stylesheet" href="main.css" type="text/css"/>
			<xsl:choose>
				<xsl:when test="@id='homePage'">
					<script type="text/javascript">setTimeout(function() { window.location.href=window.location.href; }, 23000);</script>
				</xsl:when>
				<xsl:when test="@id='headerPage'">
					<script type="text/javascript">setTimeout(function() { window.location.href=window.location.href; }, 60000);</script>
				</xsl:when>
				<xsl:when test="@id='configPage'">
					<xsl:if test="/page/config/@tab = 4">
						<script type="text/javascript">
							function setCombo(id, value)
							{
								var combo = document.getElementById(id);
								
								for (var i = 0; i &lt; combo.length; i++)
									if (value == combo.options[i].value) {
										combo.selectedIndex = i;
										return;
									}
							}
							
							function saveTime()
							{
								var now = new Date();
							
								document.getElementById('d0y').value = '' + now.getFullYear();
								setCombo('d0M', now.getMonth() + 1);
								setCombo('d0d', now.getDate());
							
								document.getElementById('d0h').value = (now.getHours() &lt; 10 ? '0' : '') + now.getHours();
								document.getElementById('d0m').value = (now.getMinutes() &lt; 10 ? '0' : '') + now.getMinutes();
								document.getElementById('d0s').value = (now.getSeconds() &lt; 10 ? '0' : '') + now.getSeconds();
							
								return true;
							}
						</script>
					</xsl:if>
					<xsl:if test="/page/config/@tab = 5">
						<xsl:if test="/page/@executed = 1"><script type="text/javascript">top.location.href='reset.htm';</script></xsl:if>
						<xsl:if test="/page/@executed = 2"><script type="text/javascript">top.location.href='shutdown.htm';</script></xsl:if>
					</xsl:if>
				</xsl:when>
			</xsl:choose>
		</head>
		<body class="mainPage">
			<xsl:attribute name="id"><xsl:value-of select="@id"/></xsl:attribute>
			<xsl:choose>
				<xsl:when test="@id='homePage'">
					<xsl:apply-templates select="current" />
					<xsl:apply-templates select="log" />
					<xsl:apply-templates select="records" />
				</xsl:when>
				<xsl:when test="@id='headerPage'">
					<div id="dateTime" style="float:right">
					<xsl:call-template name="printDate">
						<xsl:with-param name="datetime" select="@datetime" />
					</xsl:call-template>
					<br/>
					<xsl:call-template name="printTime">
						<xsl:with-param name="datetime" select="@datetime" />
					</xsl:call-template>
					</div>
					<div id="logo" style="float:left">
					</div>
					<h1>C &#183; H &#183; A &#183; R &#183; L &#183; I &#183; E</h1>
					<h2>Computerized Houseplant Anomaly Reporting, Logging and Irrigation Equipment</h2>
				</xsl:when>
				<xsl:when test="@id='recordsPage'">
					<xsl:apply-templates select="records" />
				</xsl:when>
				<xsl:when test="@id='logPage'">
					<xsl:apply-templates select="log" />
				</xsl:when>
				<xsl:when test="@id='configPage'">
					<xsl:apply-templates select="config" />
				</xsl:when>
			</xsl:choose>
		</body>
		</html>
	</xsl:template>
	<xsl:template name="pctClass">
		<xsl:param name="value" />
		<xsl:choose>
			<xsl:when test="$value &lt; 10">0</xsl:when>
			<xsl:when test="$value &lt; 35">25</xsl:when>
			<xsl:when test="$value &lt; 65">50</xsl:when>
			<xsl:when test="$value &lt; 85">75</xsl:when>
			<xsl:when test="$value &lt; 101">100</xsl:when>
			<xsl:otherwise>xxx</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	<xsl:template name="yearOption">
		<xsl:param name="year">0</xsl:param>
		<xsl:param name="select">0</xsl:param>
		<xsl:choose>
			<xsl:when test="$year = 0">
				<option xmlns="http://www.w3.org/1999/xhtml" value="0"></option>
				<xsl:call-template name="yearOption">
					<xsl:with-param name="year"><xsl:value-of select="substring(/page/@datetime,1,4)" /></xsl:with-param>
					<xsl:with-param name="select"><xsl:value-of select="$select"/></xsl:with-param>
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="$year &gt;= 2000">
				<option xmlns="http://www.w3.org/1999/xhtml">
					<xsl:attribute name="value"><xsl:value-of select="$year"/></xsl:attribute>
					<xsl:if test="number($year) = number($select)"><xsl:attribute name="selected"><xsl:value-of select="selected"/></xsl:attribute></xsl:if>
					<xsl:value-of select="$year"/>
				</option>
				<xsl:call-template name="yearOption">
					<xsl:with-param name="year"><xsl:value-of select="$year - 1"/></xsl:with-param>
					<xsl:with-param name="select"><xsl:value-of select="$select"/></xsl:with-param>
				</xsl:call-template>
			</xsl:when>
		</xsl:choose>
	</xsl:template>
	<xsl:template name="monthOption">
		<xsl:param name="month">0</xsl:param>
		<xsl:param name="select">0</xsl:param>
		<xsl:if test="$month &lt;= 12">
			<option xmlns="http://www.w3.org/1999/xhtml">
				<xsl:attribute name="value"><xsl:value-of select="$month"/></xsl:attribute>
				<xsl:if test="number($month) = number($select)"><xsl:attribute name="selected"><xsl:value-of select="selected"/></xsl:attribute></xsl:if>
				<xsl:value-of select="substring('   JanFebMarAprMayJunJulAugSepOctNovDec',1+$month*3,3)"/>
			</option>
			<xsl:call-template name="monthOption">
				<xsl:with-param name="month"><xsl:value-of select="$month + 1"/></xsl:with-param>
				<xsl:with-param name="select"><xsl:value-of select="$select"/></xsl:with-param>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>
	<xsl:template name="dayOption">
		<xsl:param name="day">0</xsl:param>
		<xsl:param name="select">0</xsl:param>
		<xsl:if test="$day &lt;= 31">
			<option xmlns="http://www.w3.org/1999/xhtml">
				<xsl:attribute name="value"><xsl:value-of select="$day"/></xsl:attribute>
				<xsl:if test="number($day) = number($select)"><xsl:attribute name="selected"><xsl:value-of select="selected"/></xsl:attribute></xsl:if>
				<xsl:if test="$day &gt; 0"><xsl:value-of select="$day"/></xsl:if>
			</option>
			<xsl:call-template name="dayOption">
				<xsl:with-param name="day"><xsl:value-of select="$day + 1"/></xsl:with-param>
				<xsl:with-param name="select"><xsl:value-of select="$select"/></xsl:with-param>
			</xsl:call-template>
		</xsl:if>
	</xsl:template>
	<xsl:template name="printDate">
		<xsl:param name="datetime" />
		<xsl:variable name="month" select="number(substring($datetime,6,2))"/>
		<xsl:value-of select="substring($datetime,9,2)" />
		<xsl:text> </xsl:text>
		<xsl:value-of select="substring('   JanFebMarAprMayJunJulAugSepOctNovDec',1+$month*3,3)" />
		<xsl:text> </xsl:text>
		'<xsl:value-of select="substring($datetime,3,2)" />
	</xsl:template>
	<xsl:template name="printTime">
		<xsl:param name="datetime" />
		<xsl:value-of select="substring($datetime,12,2)" />:<xsl:value-of select="substring($datetime,15,2)" />
	</xsl:template>
	<xsl:template name="dateInput">
		<xsl:param name="index" />
		<xsl:param name="select" />
		<xsl:param name="anyYear"/>
		<select xmlns="http://www.w3.org/1999/xhtml">
			<xsl:attribute name="name"><xsl:value-of select="concat('d', string($index), 'd')"/></xsl:attribute>
			<xsl:attribute name="id"><xsl:value-of select="concat('d', string($index), 'd')"/></xsl:attribute>
			<xsl:call-template name="dayOption"><xsl:with-param name="select" select="substring($select,9,2)"></xsl:with-param></xsl:call-template>
		</select>
		<select xmlns="http://www.w3.org/1999/xhtml">
			<xsl:attribute name="name"><xsl:value-of select="concat('d', string($index), 'M')"/></xsl:attribute>
			<xsl:attribute name="id"><xsl:value-of select="concat('d', string($index), 'M')"/></xsl:attribute>
			<xsl:call-template name="monthOption"><xsl:with-param name="select" select="substring($select,6,2)"></xsl:with-param></xsl:call-template>
		</select>
		<xsl:choose>
			<xsl:when test="$anyYear">
				<input type="text" size="4" maxlength="4" xmlns="http://www.w3.org/1999/xhtml">
					<xsl:attribute name="name"><xsl:value-of select="concat('d', string($index), 'y')"/></xsl:attribute>
					<xsl:attribute name="id"><xsl:value-of select="concat('d', string($index), 'y')"/></xsl:attribute>
					<xsl:attribute name="value">
						<xsl:if test="substring($select,1,4) != '0000'">
						<xsl:value-of select="substring($select,1,4)"/>
						</xsl:if>
					</xsl:attribute>
				</input>
			</xsl:when>
			<xsl:otherwise>
				<select xmlns="http://www.w3.org/1999/xhtml">
					<xsl:attribute name="name"><xsl:value-of select="concat('d', string($index), 'y')"/></xsl:attribute>
					<xsl:attribute name="id"><xsl:value-of select="concat('d', string($index), 'y')"/></xsl:attribute>
					<xsl:call-template name="yearOption"><xsl:with-param name="select" select="substring($select,1,4)"></xsl:with-param></xsl:call-template>
				</select>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>
	<xsl:template name="timeInput">
		<xsl:param name="index" />
		<xsl:param name="select" />
		<input type="text" size="2" maxlength="2" xmlns="http://www.w3.org/1999/xhtml">
			<xsl:attribute name="name"><xsl:value-of select="concat('d', string($index), 'h')"/></xsl:attribute>
			<xsl:attribute name="id"><xsl:value-of select="concat('d', string($index), 'h')"/></xsl:attribute>
			<xsl:attribute name="value"><xsl:value-of select="substring($select,12,2)"/></xsl:attribute>
		</input>
		<xsl:text> : </xsl:text>
		<input type="text" size="2" maxlength="2" xmlns="http://www.w3.org/1999/xhtml">
			<xsl:attribute name="name"><xsl:value-of select="concat('d', string($index), 'm')"/></xsl:attribute>
			<xsl:attribute name="id"><xsl:value-of select="concat('d', string($index), 'm')"/></xsl:attribute>
			<xsl:attribute name="value"><xsl:value-of select="substring($select,15,2)"/></xsl:attribute>
		</input>
		<xsl:text> : </xsl:text>
		<input type="text" size="2" maxlength="2" xmlns="http://www.w3.org/1999/xhtml">
			<xsl:attribute name="name"><xsl:value-of select="concat('d', string($index), 's')"/></xsl:attribute>
			<xsl:attribute name="id"><xsl:value-of select="concat('d', string($index), 's')"/></xsl:attribute>
			<xsl:attribute name="value"><xsl:value-of select="substring($select,18,2)"/></xsl:attribute>
		</input>
	</xsl:template>
	<xsl:template match="/page[@id='homePage']/current">
		<div id="statusSection" class="section" xmlns="http://www.w3.org/1999/xhtml">
			<div class="sectionTop" />
 			<div class="sectionContents">
				<h2>Current Status</h2>
				<xsl:if test="/page/@datetimebad">
				<p class="error"><div class="icon_error"/>The Real-Time Clock's battery failed in the past. No recordings can be made until you set the date/time again.</p>
				</xsl:if>
				<table id="statusTable">
					<tbody>
						<tr>
							<xsl:apply-templates select="plant" />
						</tr>
					</tbody>
				</table>
			</div>
		</div>
	</xsl:template>
	<xsl:template match="current/plant">
		<td xmlns="http://www.w3.org/1999/xhtml">
			<table class="statusEntry">
				<tbody>
					<tr class="statusImages">
						<td><div>
								<xsl:attribute name="class">
									<xsl:text>statusImage humi</xsl:text>
									<xsl:call-template name="pctClass">
										<xsl:with-param name="value" select="@humid" />
									</xsl:call-template>
								</xsl:attribute>
						</div></td>
						<td>
							<div>
								<xsl:attribute name="class">
									<xsl:text>statusImage ilum</xsl:text>
									<xsl:call-template name="pctClass">
										<xsl:with-param name="value" select="@ilum" />
									</xsl:call-template>
								</xsl:attribute>
							</div>
						</td>
					</tr>
					<tr class="statusValues">
						<td>
							<xsl:choose>
								<xsl:when test="@humid"><xsl:value-of select="@humid" />%</xsl:when>
								<xsl:otherwise>&#160;</xsl:otherwise>
							</xsl:choose>
						</td>
						<td>
							<xsl:choose>
								<xsl:when test="@ilum"><xsl:value-of select="@ilum" />%</xsl:when>
								<xsl:otherwise>&#160;</xsl:otherwise>
							</xsl:choose>
						</td>
					</tr>
				</tbody>
			</table>
			<p class="plantName">Plant <xsl:value-of select="position()"/></p>
		</td>
	</xsl:template>
	<xsl:template match="/page[@id='homePage']/log">
		<div id="logsSection" class="section" xmlns="http://www.w3.org/1999/xhtml">
			<div class="sectionTop" />
 			<div class="sectionContents">
				<div class="seeAll" style="float:right"><a href="log.php">See all</a></div>
				<h2>Event Log</h2>
				<table id="logTable" class="style1">
					<thead>
						<th>Date</th>
						<th>Event</th>
					</thead>
					<tbody>
						<xsl:apply-templates select="event" />
					</tbody>
				</table>
			</div>
		</div>
	</xsl:template>
	<xsl:template match="/page[@id='homePage']/records">
		<div id="recordsSection" class="section" xmlns="http://www.w3.org/1999/xhtml">
			<div class="sectionTop" />
 			<div class="sectionContents">
				<div class="seeAll" style="float:right"><a href="records.php">See all</a></div>
				<h2>Latest Records</h2>
				<table id="recordsTable" class="style1">
					<thead>
						<th>Date</th>
						<th colspan="2">Plant 1</th>
						<th colspan="2">Plant 2</th>
						<th colspan="2">Plant 3</th>
						<th colspan="2">Plant 4</th>
					</thead>
					<tbody>
						<xsl:apply-templates select="row" />
					</tbody>
				</table>
			</div>
		</div>
	</xsl:template>
	<xsl:template match="/page[@id='configPage']/config">
		<form name="config" method="get" action="config.php" xmlns="http://www.w3.org/1999/xhtml">
			<input type="hidden" name="t">
				<xsl:attribute name="value"><xsl:value-of select="@tab"/></xsl:attribute>
			</input>
			<div id="cfgSection" class="section">
				<div class="sectionTop" />
 				<div class="sectionContents">
					<h2>Setup</h2>
				</div>
			</div>
			<table class="tabs">
				<tbody>
					<tr>
						<td class="space1"><div></div></td>
						<td><xsl:attribute name="class">tab <xsl:if test="@tab=1">selected</xsl:if></xsl:attribute>
							<xsl:choose><xsl:when test="@tab=1"><div>Plants</div></xsl:when>
							<xsl:otherwise><a href="?t=1"><div>Plants</div></a></xsl:otherwise></xsl:choose></td>
						<td class="space2"><div></div></td>
						<td><xsl:attribute name="class">tab <xsl:if test="@tab=2">selected</xsl:if></xsl:attribute>
							<xsl:choose><xsl:when test="@tab=2"><div>Network</div></xsl:when>
							<xsl:otherwise><a href="?t=2"><div>Network</div></a></xsl:otherwise></xsl:choose></td>
						<td class="space2"><div></div></td>
						<td><xsl:attribute name="class">tab <xsl:if test="@tab=3">selected</xsl:if></xsl:attribute>
							<xsl:choose><xsl:when test="@tab=3"><div>Alerts</div></xsl:when>
							<xsl:otherwise><a href="?t=3"><div>Alerts</div></a></xsl:otherwise></xsl:choose></td>
						<td class="space2"><div></div></td>
						<td><xsl:attribute name="class">tab <xsl:if test="@tab=4">selected</xsl:if></xsl:attribute>
							<xsl:choose><xsl:when test="@tab=4"><div>Date/Time</div></xsl:when>
							<xsl:otherwise><a href="?t=4"><div>Date/Time</div></a></xsl:otherwise></xsl:choose></td>
						<td class="space2"><div></div></td>
						<td><xsl:attribute name="class">tab <xsl:if test="@tab=5">selected</xsl:if></xsl:attribute>
							<xsl:choose><xsl:when test="@tab=5"><div>Reset</div></xsl:when>
							<xsl:otherwise><a href="?t=5"><div>Reset</div></a></xsl:otherwise></xsl:choose></td>
						<td class="space3">&#160;</td>
					</tr>
				</tbody>
			</table>
			<div class="tabSection">
				<br/>
				<xsl:choose>
					<xsl:when test="@tab=1">
						<xsl:for-each select="plants/plant">
							<div class="plantConfig">
								<xsl:attribute name="style">
									float: 
									<xsl:choose><xsl:when test="(position() mod 2) = 1">left</xsl:when><xsl:otherwise>right</xsl:otherwise></xsl:choose>
								</xsl:attribute>
								<h3>Plant <xsl:value-of select="position()"/></h3>
								<p class="config">
									<label>Present:</label>
									<input type="checkbox">
										<xsl:attribute name="name">p<xsl:value-of select="position()" />i</xsl:attribute>
										<xsl:if test="(@flags mod 2)=0"><xsl:attribute name="checked">checked</xsl:attribute></xsl:if>
									</input>
								</p>
								<p class="config">
									<label>Watering start threshold:</label>
									<input type="text" size="3" maxlength="3">
										<xsl:attribute name="name">p<xsl:value-of select="position()" />l</xsl:attribute>
										<xsl:attribute name="value"><xsl:value-of select="@min"/></xsl:attribute>
									</input>
									%
								</p>
								<p class="config">
									<label>Watering stop threshold:</label>
									<input type="text" size="3" maxlength="3">
										<xsl:attribute name="name">p<xsl:value-of select="position()" />u</xsl:attribute>
										<xsl:attribute name="value"><xsl:value-of select="@max"/></xsl:attribute>
									</input>
									%
								</p>
							</div>
						</xsl:for-each>
						<br/>
						<p class="config">
							<label>Record state every:</label>
							<input type="text" name="ri" size="7" maxlength="7">
								<xsl:attribute name="value"><xsl:value-of select="recording/@interval"/></xsl:attribute>
							</input>
							(seconds)
						</p>
					</xsl:when>
					<xsl:when test="@tab=2">
						<p class="config">
							<label>MAC address:</label>
							<input type="text" name="nm" size="17" maxlength="17">
								<xsl:attribute name="value"><xsl:value-of select="net/@mac"/></xsl:attribute>
							</input>
						</p>
					</xsl:when>
					<xsl:when test="@tab=3">
						<p class="config">
							<label>Alerts server IP:</label>
							<input type="text" name="ai" size="15" maxlength="15">
								<xsl:attribute name="value"><xsl:value-of select="alerts/@ip"/></xsl:attribute>
							</input>
						</p>
						<p class="config">
							<label>Alerts server port:</label>
							<input type="text" name="ap" size="5" maxlength="5">
								<xsl:attribute name="value"><xsl:value-of select="alerts/@port"/></xsl:attribute>
							</input> (UDP)
						</p>
					</xsl:when>
					<xsl:when test="@tab=4">
						<p class="config">
							<label>Current date</label>
							<xsl:call-template name="dateInput"><xsl:with-param name="anyYear">1</xsl:with-param><xsl:with-param name="index">0</xsl:with-param><xsl:with-param name="select" select="/page/@datetime"></xsl:with-param></xsl:call-template>
						</p>
						<p class="config">
							<label>Current time</label>
							<xsl:call-template name="timeInput"><xsl:with-param name="index">0</xsl:with-param><xsl:with-param name="select" select="/page/@datetime"></xsl:with-param></xsl:call-template>
						</p>
					</xsl:when>
					<xsl:when test="@tab=5">
						<p class="reset">
							<button name="C" type="submit" value="1" onclick="return confirm('Confirm device RESET');">Software reset</button>
							<span>Restarts the device.</span>
						</p>
						<p class="reset">
							<button name="C" type="submit" value="2" onclick="return confirm('Confirm device SHUTDOWN');">Shutdown</button>
							<span>Brings the device to a state where power can be safely turned off.</span>
						</p>
						<p class="reset">
							<button name="C" type="submit" value="3" onclick="return confirm('This cannot be undone! Are you sure?');">Purge event log</button>
							<span>Deletes all entries in the device event log.</span>
						</p>
						<p class="reset">
							<button name="C" type="submit" value="4" onclick="return confirm('This cannot be undone! Are you sure?');">Purge records</button>
							<span>Deletes all plant-related records.</span>
						</p>
						<xsl:if test="/page/@executed = 3"><p class="done"><div class="icon_ok"/>All event log entries have been deleted.</p></xsl:if>
						<xsl:if test="/page/@executed = 4"><p class="done"><div class="icon_ok"/>All plant records have been deleted.</p></xsl:if>
					</xsl:when>
				</xsl:choose>
				<xsl:if test="@tab!=5">
				<div class="bottomButtons">
					<button name="C" type="submit" value="1">Save changes</button>
					<xsl:if test="@tab=4">
						<button name="C" type="submit" value="1" onclick="return saveTime();">Get time from browser (needs JavaScript)</button>
					</xsl:if>
				</div>
				</xsl:if>
			</div>
		</form>
	</xsl:template>
	<xsl:template name="pageLinks">
		<xsl:param name="page" />
		<xsl:param name="selected" />
		<xsl:param name="total" />
		<xsl:param name="qString" />
		<xsl:choose>
			<xsl:when test="$page = $selected">
				<span class="current" xmlns="http://www.w3.org/1999/xhtml"><xsl:value-of select="$page" /></span>
			</xsl:when>
			<xsl:otherwise>
				<a xmlns="http://www.w3.org/1999/xhtml">
					<xsl:attribute name="href"><xsl:value-of select="concat($qString,$page)" /></xsl:attribute>
					<xsl:value-of select="$page" />
				</a>
			</xsl:otherwise>
		</xsl:choose>
		<xsl:choose>
			<xsl:when test="($page=3) and (number($selected) &gt; 7)">
				<span class="ellipsis" xmlns="http://www.w3.org/1999/xhtml">&#183; &#183; &#183;</span>
				<xsl:call-template name="pageLinks">
					<xsl:with-param name="page" select="number($selected)-3" />
					<xsl:with-param name="selected" select="$selected" />
					<xsl:with-param name="total" select="$total" />
					<xsl:with-param name="qString" select="$qString" />
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="(number($page)=number($selected)+3) and (number($selected) &lt; (number($total)-6))">
				<span class="ellipsis" xmlns="http://www.w3.org/1999/xhtml">&#183; &#183; &#183;</span>
				<xsl:call-template name="pageLinks">
					<xsl:with-param name="page" select="number($total)-2" />
					<xsl:with-param name="selected" select="$selected" />
					<xsl:with-param name="total" select="$total" />
					<xsl:with-param name="qString" select="$qString" />
				</xsl:call-template>
			</xsl:when>
			<xsl:when test="number($page) &lt; number($total)">
				<xsl:call-template name="pageLinks">
					<xsl:with-param name="page" select="number($page)+1" />
					<xsl:with-param name="selected" select="$selected" />
					<xsl:with-param name="total" select="$total" />
					<xsl:with-param name="qString" select="$qString" />
				</xsl:call-template>
			</xsl:when>
		</xsl:choose>
	</xsl:template>
	<xsl:template name="queryHeader">
		<xsl:param name="url" />
		<xsl:param name="from" />
		<xsl:param name="to" />
		<xsl:param name="perPage" />
		<xsl:param name="page" />
		<xsl:param name="pages" />
		<xsl:param name="reverse" />
		<form class="queryParams" action="" method="get" name="query" xmlns="http://www.w3.org/1999/xhtml">
			<xsl:attribute name="action"><xsl:value-of select="$url" /></xsl:attribute>
			<input name="p" type="hidden">
				<xsl:attribute name="value"><xsl:value-of select="$page"/></xsl:attribute>
			</input>
			<label class="first">Entries/page:</label>
			<select name="n">
				<option value="15"><xsl:if test="$perPage = 15"><xsl:attribute name="selected" value="selected"></xsl:attribute></xsl:if>15</option>
				<option value="25"><xsl:if test="$perPage = 25"><xsl:attribute name="selected" value="selected"></xsl:attribute></xsl:if>25</option>
				<option value="50"><xsl:if test="$perPage = 50"><xsl:attribute name="selected" value="selected"></xsl:attribute></xsl:if>50</option>
				<option value="100"><xsl:if test="$perPage = 100"><xsl:attribute name="selected" value="selected"></xsl:attribute></xsl:if>100</option>
				<option value="200"><xsl:if test="$perPage = 200"><xsl:attribute name="selected" value="selected"></xsl:attribute></xsl:if>200</option>
				<option value="500"><xsl:if test="$perPage = 500"><xsl:attribute name="selected" value="selected"></xsl:attribute></xsl:if>500</option>
			</select>
			<label>Timespan:</label>
			<xsl:call-template name="dateInput"><xsl:with-param name="index">0</xsl:with-param><xsl:with-param name="select" select="$from"></xsl:with-param></xsl:call-template>
			<label class="dash">-</label>
			<xsl:call-template name="dateInput"><xsl:with-param name="index">1</xsl:with-param><xsl:with-param name="select" select="$to"></xsl:with-param></xsl:call-template>
			<label>Order:</label>
			<select name="r">
				<option value="0"><xsl:if test="$reverse = 0"><xsl:attribute name="selected" value="selected"></xsl:attribute></xsl:if>Oldest first</option>
				<option value="1"><xsl:if test="$reverse = 1"><xsl:attribute name="selected" value="selected"></xsl:attribute></xsl:if>Newest first</option>
			</select>
			<button class="export" name="C" type="submit" value="2">CSV</button>
			<button class="applyQuery" name="C" type="submit" value="1">Apply</button>
		</form>
		<p class="pageLinks" xmlns="http://www.w3.org/1999/xhtml">
			Page:
			<xsl:variable name="qString">
				<xsl:value-of select="$url" />
				<xsl:text>?n=</xsl:text>
				<xsl:value-of select="$perPage" />
				<xsl:text>&amp;r=</xsl:text>
				<xsl:value-of select="$reverse" />
				<xsl:text>&amp;d0d=</xsl:text>
				<xsl:value-of select="number(substring($from,9,2))" />
				<xsl:text>&amp;d0M=</xsl:text>
				<xsl:value-of select="number(substring($from,6,2))" />
				<xsl:text>&amp;d0y=</xsl:text>
				<xsl:value-of select="number(substring($from,1,4))" />
				<xsl:text>&amp;d1d=</xsl:text>
				<xsl:value-of select="number(substring($to,9,2))" />
				<xsl:text>&amp;d1M=</xsl:text>
				<xsl:value-of select="number(substring($to,6,2))" />
				<xsl:text>&amp;d1y=</xsl:text>
				<xsl:value-of select="number(substring($to,1,4))" />
				<xsl:text>&amp;p=</xsl:text>
			</xsl:variable>
			<xsl:if test="number($page) &gt; 1"><a><xsl:attribute name="href"><xsl:value-of select="concat($qString,number($page)-1)" /></xsl:attribute>&lt;&lt;</a></xsl:if>
			<xsl:call-template name="pageLinks">
				<xsl:with-param name="page">1</xsl:with-param>
				<xsl:with-param name="selected" select="$page" />
				<xsl:with-param name="total" select="$pages" />
				<xsl:with-param name="qString" select="$qString" />
			</xsl:call-template>
			<xsl:if test="number($page) &lt; number($pages)"><a><xsl:attribute name="href"><xsl:value-of select="concat($qString,number($page)+1)" /></xsl:attribute>&gt;&gt;</a></xsl:if>
		</p>
	</xsl:template>
	<xsl:template match="/page[@id='recordsPage']/records">
		<div id="recordsSection" class="section" xmlns="http://www.w3.org/1999/xhtml">
			<div class="sectionTop" />
			<div class="sectionContents">
				<h2>Records</h2>
				<xsl:call-template name="queryHeader">
					<xsl:with-param name="url">records.php</xsl:with-param>
					<xsl:with-param name="from" select="@from" />
					<xsl:with-param name="to" select="@to" />
					<xsl:with-param name="page" select="@page" />
					<xsl:with-param name="perPage" select="@perPage" />
					<xsl:with-param name="pages" select="@pages" />
					<xsl:with-param name="reverse" select="@reverse" />
				</xsl:call-template>
				<table id="recordsTable" class="style1">
					<thead>
						<th>Date</th>
						<th colspan="2">Plant 1</th>
						<th colspan="2">Plant 2</th>
						<th colspan="2">Plant 3</th>
						<th colspan="2">Plant 4</th>
					</thead>
					<tbody>
						<xsl:apply-templates select="row" />
					</tbody>
				</table>
			</div>
		</div>
	</xsl:template>
	<xsl:template match="/page[@id='logPage']/log">
		<div id="logsSection" class="section" xmlns="http://www.w3.org/1999/xhtml">
			<div class="sectionTop" />
			<div class="sectionContents">
				<h2>Event Log</h2>
				<xsl:call-template name="queryHeader">
					<xsl:with-param name="url">log.php</xsl:with-param>
					<xsl:with-param name="from" select="@from" />
					<xsl:with-param name="to" select="@to" />
					<xsl:with-param name="page" select="@page" />
					<xsl:with-param name="perPage" select="@perPage" />
					<xsl:with-param name="pages" select="@pages" />
					<xsl:with-param name="reverse" select="@reverse" />
				</xsl:call-template>
				<table id="logTable" class="style1">
					<thead>
						<th>Date</th>
						<th>Event</th>
					</thead>
					<tbody>
						<xsl:apply-templates select="event" />
					</tbody>
				</table>
			</div>
		</div>
	</xsl:template>
	<xsl:template match="records/row">
		<tr xmlns="http://www.w3.org/1999/xhtml">
			<xsl:attribute name="class">
				<xsl:choose>
					<xsl:when test="position() mod 2">alt0</xsl:when>
					<xsl:otherwise>alt1</xsl:otherwise>
				</xsl:choose>
			</xsl:attribute>
			<td class="hdrCol">
				<xsl:if test="substring(@datetime,1,10) != substring(preceding-sibling::row[1]/@datetime,1,10)">
					<xsl:call-template name="printDate">
						<xsl:with-param name="datetime" select="@datetime" />
					</xsl:call-template>
				</xsl:if>
				&#160;
				<xsl:call-template name="printTime">
					<xsl:with-param name="datetime" select="@datetime" />
				</xsl:call-template>
			</td>
			<xsl:apply-templates select="plant" />
		</tr>
	</xsl:template>
	<xsl:template match="records/row/plant">		
		<td xmlns="http://www.w3.org/1999/xhtml">
			<xsl:attribute name="class">
				<xsl:text>humi humi</xsl:text>
				<xsl:call-template name="pctClass">
					<xsl:with-param name="value" select="@humid" />
				</xsl:call-template>
			</xsl:attribute>
			<xsl:choose>
				<xsl:when test="@humid"><xsl:value-of select="@humid" />%</xsl:when>
				<xsl:otherwise>-</xsl:otherwise>
			</xsl:choose>
		</td>
		<td xmlns="http://www.w3.org/1999/xhtml">
			<xsl:attribute name="class">
				<xsl:text>ilum ilum</xsl:text>
				<xsl:call-template name="pctClass">
					<xsl:with-param name="value" select="@ilum" />
				</xsl:call-template>
			</xsl:attribute>
			<xsl:choose>
				<xsl:when test="@ilum"><xsl:value-of select="@ilum" />%</xsl:when>
				<xsl:otherwise>-</xsl:otherwise>
			</xsl:choose>
		</td>
	</xsl:template>
	<xsl:template match="log/event">
		<tr xmlns="http://www.w3.org/1999/xhtml">
			<xsl:attribute name="class">
				<xsl:choose>
					<xsl:when test="position() mod 2">alt0</xsl:when>
					<xsl:otherwise>alt1</xsl:otherwise>
				</xsl:choose>
			</xsl:attribute>
			<td class="hdrCol">
				<xsl:if test="substring(@datetime,1,10) != substring(preceding-sibling::event[1]/@datetime,1,10)">
					<xsl:call-template name="printDate">
						<xsl:with-param name="datetime" select="@datetime" />
					</xsl:call-template>
				</xsl:if>
				&#160;
				<xsl:call-template name="printTime">
					<xsl:with-param name="datetime" select="@datetime" />
				</xsl:call-template>
			</td>
			<td>
				<xsl:attribute name="class">
					<xsl:text>eventCol </xsl:text>
					<xsl:choose>
						<xsl:when test="@code &lt; 64">info</xsl:when>
						<xsl:when test="@code &lt; 128">warning</xsl:when>
						<xsl:otherwise>error</xsl:otherwise>
					</xsl:choose>
				</xsl:attribute>
				<xsl:choose>
					<xsl:when test="@code = 1">C.H.A.R.L.I.E. activated</xsl:when>
					<xsl:when test="@code = 2">Started watering plant <xsl:value-of select="@data" /></xsl:when>
					<xsl:when test="@code = 3">Done watering plant <xsl:value-of select="@data" /></xsl:when>
					<xsl:when test="@code = 65">C.H.A.R.L.I.E. shutting down</xsl:when>
					<xsl:when test="@code = 66">C.H.A.R.L.I.E. restarting</xsl:when>
					<xsl:when test="@code = 129">RTC battery failed sometime in the past</xsl:when>
				</xsl:choose>
			</td>
		</tr>
	</xsl:template>
</xsl:stylesheet>
