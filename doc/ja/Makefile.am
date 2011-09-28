docdir = $(pkgdatadir)/doc
doc_jadir = $(docdir)/ja

# Makefile for Sphinx documentation
#

# You can set these variables from the command line.
SPHINXOPTS    =
PAPER         =
BUILDDIR      = .

# Internal variables.
PAPEROPT_a4     = -D latex_paper_size=a4
PAPEROPT_letter = -D latex_paper_size=letter
# ALLSPHINXOPTS   = -d $(BUILDDIR)/doctrees $(PAPEROPT_$(PAPER)) $(SPHINXOPTS) source
ALLSPHINXOPTS   = -d $(BUILDDIR)/doctrees $(SPHINXOPTS) source

RUN_SPHINX_BUILD = env VERSION=$(VERSION) $(SPHINX_BUILD)

.PHONY: help clean html dirhtml pickle json htmlhelp qthelp latex changes linkcheck doctest

help:
	@echo "Please use \`make <target>' where <target> is one of"
	@echo "  html      to make standalone HTML files"
	@echo "  dirhtml   to make HTML files named index.html in directories"
	@echo "  pickle    to make pickle files"
	@echo "  json      to make JSON files"
	@echo "  htmlhelp  to make HTML files and a HTML help project"
	@echo "  qthelp    to make HTML files and a qthelp project"
	@echo "  latex     to make LaTeX files, you can set PAPER=a4 or PAPER=letter"
	@echo "  changes   to make an overview of all changed/added/deprecated items"
	@echo "  linkcheck to check all external links for integrity"
	@echo "  doctest   to run all doctests embedded in the documentation (if enabled)"

clean-local:
	-rm -rf $(BUILDDIR)/doctree
	-rm -rf $(BUILDDIR)/html
	-rm -rf $(BUILDDIR)/dirhtml
	-rm -rf $(BUILDDIR)/pickle
	-rm -rf $(BUILDDIR)/json
	-rm -rf $(BUILDDIR)/htmlhelp
	-rm -rf $(BUILDDIR)/qthelp
	-rm -rf $(BUILDDIR)/latex
	-rm -rf $(BUILDDIR)/changes
	-rm -rf $(BUILDDIR)/linkcheck
	-rm -rf $(BUILDDIR)/doctest
	-rm -rf $(BUILDDIR)/pdf

html-local:
	$(RUN_SPHINX_BUILD) -b html $(ALLSPHINXOPTS) $(BUILDDIR)/html
	@echo
	@echo "Build finished. The HTML pages are in $(BUILDDIR)/html."

dirhtml:
	$(RUN_SPHINX_BUILD) -b dirhtml $(ALLSPHINXOPTS) $(BUILDDIR)/dirhtml
	@echo
	@echo "Build finished. The HTML pages are in $(BUILDDIR)/dirhtml."

pickle:
	$(RUN_SPHINX_BUILD) -b pickle $(ALLSPHINXOPTS) $(BUILDDIR)/pickle
	@echo
	@echo "Build finished; now you can process the pickle files."

json:
	$(RUN_SPHINX_BUILD) -b json $(ALLSPHINXOPTS) $(BUILDDIR)/json
	@echo
	@echo "Build finished; now you can process the JSON files."

htmlhelp:
	$(RUN_SPHINX_BUILD) -b htmlhelp $(ALLSPHINXOPTS) $(BUILDDIR)/htmlhelp
	@echo
	@echo "Build finished; now you can run HTML Help Workshop with the" \
	      ".hhp project file in $(BUILDDIR)/htmlhelp."

qthelp:
	$(RUN_SPHINX_BUILD) -b qthelp $(ALLSPHINXOPTS) $(BUILDDIR)/qthelp
	@echo
	@echo "Build finished; now you can run "qcollectiongenerator" with the" \
	      ".qhcp project file in $(BUILDDIR)/qthelp, like this:"
	@echo "# qcollectiongenerator $(BUILDDIR)/qthelp/groonga.qhcp"
	@echo "To view the help file:"
	@echo "# assistant -collectionFile $(BUILDDIR)/qthelp/groonga.qhc"

latex:
	$(RUN_SPHINX_BUILD) -b latex $(ALLSPHINXOPTS) $(BUILDDIR)/latex
	@echo
	@echo "Build finished; the LaTeX files are in $(BUILDDIR)/latex."
	@echo "Run \`make all-pdf' or \`make all-ps' in that directory to" \
	      "run these through (pdf)latex."

changes:
	$(RUN_SPHINX_BUILD) -b changes $(ALLSPHINXOPTS) $(BUILDDIR)/changes
	@echo
	@echo "The overview file is in $(BUILDDIR)/changes."

linkcheck:
	$(RUN_SPHINX_BUILD) -b linkcheck $(ALLSPHINXOPTS) $(BUILDDIR)/linkcheck
	@echo
	@echo "Link check complete; look for any errors in the above output " \
	      "or in $(BUILDDIR)/linkcheck/output.txt."

doctest:
	$(RUN_SPHINX_BUILD) -b doctest $(ALLSPHINXOPTS) $(BUILDDIR)/doctest
	@echo "Testing of doctests in the sources finished, look at the " \
	      "results in $(BUILDDIR)/doctest/output.txt."

pdf-local:
	$(RUN_SPHINX_BUILD) -b pdf $(ALLSPHINXOPTS) $(BUILDDIR)/pdf
	@echo
	@echo "Build finished. The PDF files are in $(BUILDDIR)/pdf."

if ENABLE_DOCUMENT
dist-hook:
	@touch $(distdir)/html-build-stamp

nobase_dist_doc_ja_DATA =			\
	$(source_files)				\
	$(html_files)

# find source -type f -not -name '*.pyc' | sort | sed -e 's,^,\t,g'
source_files =					\
	source/images/storage-mode.png		\
	source/images/storage-mode.svg		\
	source/images/wrapper-mode.png		\
	source/images/wrapper-mode.svg		\
	source/characteristic.rst		\
	source/conf.py				\
	source/developer.rst			\
	source/developer/debug.rst		\
	source/developer/release.rst		\
	source/index.rst			\
	source/install.rst			\
	source/news.rst				\
	source/reference.rst			\
	source/userguide.rst			\
	source/userguide/storage.rst		\
	source/userguide/wrapper.rst

# find html -type f | sort | sed -e 's,^,\t,g'
html_files =					\
	html/.buildinfo				\
	html/_images/storage-mode.png		\
	html/_images/wrapper-mode.png		\
	html/_sources/characteristic.txt	\
	html/_sources/developer.txt		\
	html/_sources/developer/debug.txt	\
	html/_sources/developer/release.txt	\
	html/_sources/index.txt			\
	html/_sources/install.txt		\
	html/_sources/news.txt			\
	html/_sources/reference.txt		\
	html/_sources/userguide.txt		\
	html/_sources/userguide/storage.txt	\
	html/_sources/userguide/wrapper.txt	\
	html/_static/basic.css			\
	html/_static/default.css		\
	html/_static/doctools.js		\
	html/_static/file.png			\
	html/_static/jquery.js			\
	html/_static/minus.png			\
	html/_static/plus.png			\
	html/_static/pygments.css		\
	html/_static/searchtools.js		\
	html/_static/sidebar.js			\
	html/_static/translations.js		\
	html/_static/underscore.js		\
	html/characteristic.html		\
	html/developer.html			\
	html/developer/debug.html		\
	html/developer/release.html		\
	html/genindex.html			\
	html/index.html				\
	html/install.html			\
	html/news.html				\
	html/objects.inv			\
	html/reference.html			\
	html/search.html			\
	html/searchindex.js			\
	html/userguide.html			\
	html/userguide/storage.html		\
	html/userguide/wrapper.html

$(html_files): html-build-stamp
html-build-stamp:
	$(MAKE) html
endif
