<?php
$output = '
typedef struct {
    size_t size;
    const uint8_t data[];
} binfile_t;

';

chdir(__DIR__.'/images');

foreach (glob('*.png') as $file) {
    $data = file_get_contents($file);
    $size = strlen($data);
    $hex = [];
    for ($x = 0; $x < $size; ++$x) {
        $hex[] = sprintf('0x%02X', ord($data[$x]));
    }
    $hexdata = wordwrap(implode(', ', $hex), 100);
    $struct_name = preg_replace('/[^a-z0-9]+/i', '_', basename($file, '.png'));

    $output .= "static const binfile_t $struct_name = {.size = $size, .data = {\n$hexdata\n} };";
    $output .= "\n\n";
}

file_put_contents('../main/images.h', $output);
