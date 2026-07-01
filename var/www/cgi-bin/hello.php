<?php
$query = isset($_SERVER['QUERY_STRING']) ? $_SERVER['QUERY_STRING'] : '';
$name = '';
if ($query) {
    parse_str($query, $params);
    $name = isset($params['name']) ? htmlspecialchars($params['name']) : '';
}


echo "<html><body>";
echo "<h1>Hello from PHP CGI</h1>";
if ($name)
    echo "<p>Hello, " . $name . "!</p>";
else
    echo "<p>Try: /cgi-bin/hello.php?name=John</p>";
echo "<p>Method: " . $_SERVER['REQUEST_METHOD'] . "</p>";
echo "<p>Server: " . $_SERVER['SERVER_NAME'] . ":" . $_SERVER['SERVER_PORT'] . "</p>";
echo "</body></html>";
?>
