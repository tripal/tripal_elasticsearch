<?php

/**
 * Created by PhpStorm.
 * User: mingchen
 * Date: 1/29/17
 * Time: 4:46 PM
 */
class ElasticTokenFilters
{
    public static function standard()
    {
        return [
            "type" => "standard"
        ];
    }

    public static function word_delimiter ($generate_word_parts = true,
                                   $generate_number_parts = true,
                                   $catenate_words = false,
                                   $catenate_numbers = false,
                                   $catenate_all = false,
                                   $split_on_case_change = true,
                                   $preserve_original = false,
                                   $split_on_numerics = false,
                                   $stem_english_possessive = true)
    {
        return [
            "type" => "word_delimiter",
            "generate_word_parts" => $generate_word_parts,
            "generate_number_parts" => $generate_number_parts,
            "catenate_words" => $catenate_words,
            "catenate_numbers" => $catenate_numbers,
            "catenate_all" => $catenate_all,
            "split_on_case_change" => $split_on_case_change,
            "preserve_original" => $preserve_original,
            "split_on_numerics" => $split_on_numerics,
            "stem_english_possessive" => $stem_english_possessive
        ];
    }

    public static function lowercase ()
    {
        return [
            "type" => "lowercase"
        ];
    }

    public static function uppercase ()
    {
        return [
            "type" => "uppercase"
        ];
    }

    public static function stop ($ignore_case = false, $remove_trailing = true, array $stop_words = [])
    {
        return [
            "type" => "stop",
            "ignore_case" => $ignore_case,
            "remove_trailing" => $remove_trailing,
            "stop_words" => $stop_words
        ];
    }

}