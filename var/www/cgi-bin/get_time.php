#!/usr/bin/env php
<?php
$tz = 'Europe/Paris';
date_default_timezone_set($tz);

$now    = time();
$date   = date('Y-m-d', $now);
$heure  = date('H:i:s', $now);
$jour   = date('l', $now);
$semaine = date('W', $now);

echo "Content-Type: text/html\r\n\r\n";
echo "<html><head><title>Heure serveur GET</title></head><body>";
echo "<h1>GET - Heure du serveur</h1>";
echo "<table border='1' cellpadding='8'>";
echo "<tr><td>Date</td><td><strong>" . $date . "</strong></td></tr>";
echo "<tr><td>Heure</td><td><strong>" . $heure . "</strong></td></tr>";
echo "<tr><td>Jour</td><td>" . $jour . "</td></tr>";
echo "<tr><td>Semaine</td><td>n&deg;" . $semaine . "</td></tr>";
echo "<tr><td>Timestamp Unix</td><td>" . $now . "</td></tr>";
echo "<tr><td>Fuseau horaire</td><td>" . $tz . "</td></tr>";
echo "</table>";
echo "</body></html>";
?>
