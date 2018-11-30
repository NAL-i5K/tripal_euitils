<?php


class BiosamplePropertyLookup {


  public function lookupAll($file = 'https://www.ncbi.nlm.nih.gov/biosample/docs/attributes/?format=xml') {

    $terms = [];

    $xml = file_get_contents($file);

    $x = simplexml_load_string($xml);


    $children = $x->children();


    foreach ($children as $child) {

      $term_label = $child->Name;
      $harmonized_name = $child->HarmonizedName;
      $definition = $child->Description;

      //we could look at Package if we care where the term comes from.

      $terms[(string) $harmonized_name] = [
        'label' => (string) $term_label,
        'def' => (string) $definition,
      ];

    }

    return $terms;

  }


}