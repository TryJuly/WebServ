#!/usr/bin/env php
<?php
$method = isset($_SERVER['REQUEST_METHOD']) ? $_SERVER['REQUEST_METHOD'] : '';
$cl     = isset($_SERVER['CONTENT_LENGTH']) ? intval($_SERVER['CONTENT_LENGTH']) : 0;

$body = '';
if ($method === 'POST' && $cl > 0)
    $body = fread(STDIN, $cl);

$fields = array();
parse_str($body, $fields);

$a  = isset($fields['a'])  ? $fields['a']  : null;
$b  = isset($fields['b'])  ? $fields['b']  : null;
$op = isset($fields['op']) ? $fields['op'] : null;

$result = null;
$error  = '';

if ($a !== null && $b !== null && $op !== null) {
    $a = floatval($a);
    $b = floatval($b);
    if ($op === 'add')      $result = $a + $b;
    elseif ($op === 'sub')  $result = $a - $b;
    elseif ($op === 'mul')  $result = $a * $b;
    elseif ($op === 'div') {
        if ($b == 0) $error = "Division par zero impossible.";
        else         $result = $a / $b;
    } else {
        $error = "Operation inconnue : " . htmlspecialchars($op);
    }
}

echo "Content-Type: text/html\r\n\r\n";
echo "<html><head><title>Calculatrice POST</title></head><body>";
echo "<h1>POST - Calculatrice</h1>";
echo "<p>Envoie <code>a=10&amp;op=add&amp;b=5</code> en POST (application/x-www-form-urlencoded).</p>";
if ($error) {
    echo "<p style='color:red'>Erreur : " . $error . "</p>";
} elseif ($result !== null) {
    echo "<p>" . $a . " " . $op . " " . $b . " = <strong>" . $result . "</strong></p>";
} else {
    echo "<p>Aucun parametre recu.</p>";
}
echo "<h2>Corps brut :</h2><pre>" . htmlspecialchars($body) . "</pre>";
echo "</body></html>";
?>
