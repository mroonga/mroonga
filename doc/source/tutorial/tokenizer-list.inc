.. list-table:: Available tokenizers
   :header-rows: 1

   * - Tokenizer
     - Description

   * - ``off``
     - (``Deprecated``) It does not tokenize at all. Use ``off`` if you want to treat
       ``content`` as is. For example, this value is used for prefix
       search.

   * - ``none``
     - .. versionadded:: 8.07 It does not tokenize at all. Use ``none`` if you want to treat ``content`` as is. For example, this value is used for prefix search.

   * - ``TokenBigram``
     - It tokenises in bigram. But continuous alphabets, numbers or
       symbols are treated as a token. So there can exist tokes with 3
       letters or more. It is to reduce noises.

       This is the default value.

   * - ``TokenMecab``
     - It tokenises using MeCab. Groonga should be built with MeCab
       support.

   * - ``TokenBigramSplitSymbol``
     - It tokenises in bigram. Unlike ``TokenBigram``, continuous
       symbols are not treated as a token, but tokenised in bigram.

       When you use ``TokenBigramSplitSymbol`` instead of
       ``TokenBigram``, ``"!?"`` can match ``"!?!?!?"`` in ``"Is it
       really!?!?!?"``. But when you use ``TokenBigram``, only
       ``"!?!?!?"`` can match as well.

   * - ``TokenBigramSplitSymbolAlpha``
     - It tokenise in bigram. In addition to ``TokenBigramSplitSymbol``,
       continuous alphabets are not treated as a token either, but
       tokenised in bigram.

       When you use ``TokenBigramSplitSymbolAlpha`` instead of
       ``TokenBigram``, ``"real"`` can match ``"Is it really?"``. But
       when you use ``TokenBigram``, only ``"really"`` can match as well.

   * - ``TokenBigramSplitSymbolAlphaDigit``
     - It tokenise in bigram. In addition to
       ``TokenBigramSplitSymbolAlpha``, continuous numbers are not treated
       as a token either, but tokenised in bigram. So any kind of
       characters are treated equally in bigram.

       When you use ``TokenBigramSplitSymbolAlphaDigit`` instead of
       ``TokenBigram``, ``"567"`` can match ``"090-0123-4567"``. But
       when you use ``TokenBigram``, only ``"4567"`` can match as well.

   * - ``TokenBigramIgnoreBlank``
     - It tokenise in bigram. Unlike ``TokenBigram``, it ignores white
       spaces.

       When you use ``TokenBigramIgnoreBlank`` instead of ``TokenBigram``,
       ``"みなさん"`` can match ``"み な さ ん 注 目"``. But when you use
       ``TokenBigram``, only ``"み な さ ん"`` can match as well.

   * - ``TokenBigramIgnoreBlankSplitSymbol``
     - It tokenise in bigram. Unlike ``TokenBigramSplitSymbol``, it
       ignores white spaces.

       When you use ``TokenBigramIgnoreBlankSplitSymbol`` instead of
       ``TokenBigramSplitSymbol``, ``"???"`` can match ``"! ?
       ???"``. But when you use ``TokenBigramSplitSymbol``, only ``"?
       ??"`` can match as well.

   * - ``TokenBigramIgnoreBlankSplitSymbolAlpha``
     - It tokenise in bigram. Unlike ``TokenBigramSplitSymbolAlpha``,
       it ignores white spaces.

       When you use ``TokenBigramIgnoreBlankSplitSymbolAlpha`` instead
       of ``TokenBigramSplitSymbolAlpha``, ``"ama"`` can match ``"I am
       a pen."``. But when you use ``TokenBigramSplitSymbolAlpha``,
       only ``"am a"`` can match as well.

   * - ``TokenBigramIgnoreBlankSplitSymbolAlphaDigit``
     - It tokenise in bigram. Unlike
       ``TokenBigramSplitSymbolAlphaDigit``, it ignores white spaces.

       When you use ``TokenBigramIgnoreBlankSplitSymbolAlphaDigit``
       instead of ``TokenBigramSplitSymbolAlphaDigit``, ``"9001"`` can
       match ``"090 0123 4567"``. But when you use
       ``TokenBigramSplitSymbolAlphaDigit``, only ``"90 01"`` can
       match as well.

   * - ``TokenDelimit``
     - It tokenise by splitting with a white space.

       ``"movie horror topic"`` will be tokenised as ``"movie"``,
       ``"horror"``, ``"topic"``.

   * - ``TokenDelimitNull``
     - It tokenise by splitting with a null character (``\\0``).

       ``"movie\\0horror\\0topic"`` will be tokenised as ``"movie"``,
       ``"horror"``, ``"topic"``.

   * - ``TokenUnigram``
     - It tokenises in unigram. But continuous alphabets, numbers or
       symbols are treated as a token. So there can exist tokes with 2
       letters or more. It is to reduce noises.

   * - ``TokenTrigram``
     - It tokenises in trigram. But continuous alphabets, numbers or
       symbols are treated as a token. So there can exist tokes with 4
       letters or more. It is to reduce noises.
