<?php

/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 1/29/17
 * Time: 4:14 PM
 */
class ElasticCharacterFilters
{
    public function html_strip (array $escaped_tags = [])
    {
        return [
            "type" => "html_strip",
            "escaped_tags" => $escaped_tags
        ];
    }

    public function  mapping (array $mappings = [])
    {
        return [
            "type" => "mappings",
            "mappings" => $mappings
        ];
    }

    public function pattern_replace ($pattern, $replacement)
    {
        return [
            "type" => "pattern_replacement",
            "pattern" => $pattern,
            "replacement" => $replacement
        ];
    }

    public function merge_character_filters(array $character_filter_list)
    {
        $merged_character_filter = [];

        foreach ($character_filter_list as $character_filter)
        {
            $merged_character_filter[$character_filter] = [
                "my_" . $character_filter => [
                    $this->$character_filter
                ]
            ];
        }

        return $merged_character_filter;
    }

}