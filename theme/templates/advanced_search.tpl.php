<div>
    <h4>Advanced Search Tips</h4>
    <p>Wildcard search with <code>*</code>. Examples:</p>

    <pre>
genom* sequence (matches: genome, genomic, genomics ...)
Lir*dron tulipifera (matches: Liriodendron tulipifera)
</pre>

    <p>
        Fuzzy search: When you don't know how to exactly spell the keywords,
        you can use fuzzy search. Fuzzy search allows you to search for similar
        words. You use the <code>~</code> character at the end of your keyword for fuzzy
        search (<code>keyword~</code>). Examples:
    </p>

    <pre>
<code>sequeeence~</code> (matches: sequence)
<code>Alnus rhmifolia~</code> (matches: Alnus rhombifolia)
</pre>

    <p>
        Regular expression search: wrapping keywords with forward slash (/).
        Examples:
    </p>

    <pre>/transcriptom[a-z]+/ (matches: transcriptome, transcriptomes, transcriptomics ...)</pre>

    <p>
        Boolean operators: + and -. + means must present; - means must not
        present. Examples:
    </p>

    <pre>
+"green ash" +transcriptome -genome (excludes the word genome)
+"green ash" -transcriptome +genome (includes the word genome)
</pre>

    <p>AND, OR, NOT operator and combination search. Examples:</p>

    <pre>"heat stress" AND ("Castanea mollissima" OR "green ash") NOT "heat shock"</pre>
</div>