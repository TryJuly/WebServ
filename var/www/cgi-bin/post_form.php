#!/usr/bin/env php
<?php
$method = isset($_SERVER['REQUEST_METHOD']) ? $_SERVER['REQUEST_METHOD'] : '';
$cl     = isset($_SERVER['CONTENT_LENGTH']) ? intval($_SERVER['CONTENT_LENGTH']) : 0;
$ctype  = isset($_SERVER['CONTENT_TYPE'])   ? $_SERVER['CONTENT_TYPE']           : '';

$body = '';
if ($method === 'POST' && $cl > 0)
    $body = fread(STDIN, $cl);

$fields = array();
parse_str($body, $fields);

$name    = isset($fields['name'])    ? htmlspecialchars(trim($fields['name']))    : '';
$email   = isset($fields['email'])   ? htmlspecialchars(trim($fields['email']))   : '';
$message = isset($fields['message']) ? htmlspecialchars(trim($fields['message'])) : '';

$errors = array();
if (!$name)    $errors[] = "Le champ 'name' est vide.";
if (!$email || !filter_var($fields['email'], FILTER_VALIDATE_EMAIL))
    $errors[] = "L'adresse email est invalide.";
if (!$message) $errors[] = "Le champ 'message' est vide.";

echo "Content-Type: text/html\r\n\r\n";
echo "<html><head><title>Formulaire POST</title></head><body>";
echo "<h1>POST - Traitement de formulaire</h1>";
if ($errors) {
    echo "<h2 style='color:red'>Erreurs :</h2><ul>";
    foreach ($errors as $e) echo "<li>" . $e . "</li>";
    echo "</ul>";
} else {
    echo "<p style='color:green'><strong>Formulaire valide !</strong></p>";
    echo "<table border='1' cellpadding='6'>";
    echo "<tr><td>Nom</td><td>" . $name . "</td></tr>";
    echo "<tr><td>Email</td><td>" . $email . "</td></tr>";
    echo "<tr><td>Message</td><td>" . $message . "</td></tr>";
    echo "</table>";
}
echo "<h2>Corps brut :</h2><pre>" . htmlspecialchars($body) . "</pre>";
echo "</body></html>";
?>
