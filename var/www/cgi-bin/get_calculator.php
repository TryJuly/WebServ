#!/usr/bin/env php
<?php
$qs = isset($_SERVER['QUERY_STRING']) ? $_SERVER['QUERY_STRING'] : '';
parse_str($qs, $p);

$a  = isset($p['a'])  ? floatval($p['a'])  : null;
$b  = isset($p['b'])  ? floatval($p['b'])  : null;
$op = isset($p['op']) ? $p['op']           : null;

$result  = null;
$error   = '';

if ($a !== null && $b !== null && $op !== null) {
    if ($op === 'add')      $result = $a + $b;
    elseif ($op === 'sub')  $result = $a - $b;
    elseif ($op === 'mul')  $result = $a * $b;
    elseif ($op === 'div') {
        if ($b == 0) $error = "Division par zero impossible.";
        else         $result = $a / $b;
    } else {
        $error = "Operation inconnue : " . htmlspecialchars($op) . ". Utilise add, sub, mul, div.";
    }
}

echo "Content-Type: text/html\r\n\r\n";
echo "<html><head><title>Calculatrice GET</title></head><body>";
echo "<h1>GET - Calculatrice</h1>";
echo "<p>Usage : <code>?a=10&amp;op=add&amp;b=5</code></p>";
if ($error) {
    echo "<p style='color:red'>Erreur : " . $error . "</p>";
} elseif ($result !== null) {
    echo "<p>" . $a . " " . $op . " " . $b . " = <strong>" . $result . "</strong></p>";
} else {
    echo "<p>Renseigne <code>a</code>, <code>b</code> et <code>op</code>.</p>";
}
echo "</body></html>";
?>
