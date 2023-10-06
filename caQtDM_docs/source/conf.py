# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "caQtDM"
copyright = "2007, Anton Mezger, Helge Brands"  # GPL3
author = "Anton Mezger, Helge Brands"
release = "4.4.0"

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = """
    sphinx.ext.autodoc
    sphinx.ext.autosummary
    sphinx.ext.githubpages
    sphinx.ext.todo
    sphinx.ext.viewcode
""".split()

templates_path = ["_templates"]
source_suffix = ".rst .md".split()
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

# html_theme = 'alabaster'
# html_theme = 'sphinxdoc'
html_theme = "pydata_sphinx_theme"
html_static_path = ["_static"]
