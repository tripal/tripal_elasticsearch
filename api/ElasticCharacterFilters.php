<?php

/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 1/29/17
 * Time: 4:14 PM
 */
class ElasticCharacterFilters
{
    public static function html_strip(array $escaped_tags = [])
    {
        return [
            "type" => "html_strip",
            "escaped_tags" => $escaped_tags
        ];
    }

    public static function mapping(array $mappings = [])
    {
        return [
            "type" => "mappings",
            "mappings" => $mappings
        ];
    }

    public static function pattern_replace($pattern, $replacement)
    {
        return [
            "type" => "pattern_replacement",
            "pattern" => $pattern,
            "replacement" => $replacement
        ];
    }

}
