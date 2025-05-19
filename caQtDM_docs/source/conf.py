# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = "caQtDM"
copyright = "2012-2025, Anton Mezger, Helge Brands"  # GPL3
author = "Anton Mezger, Helge Brands"
release = "4.5.0"
root_doc = "documentation"


# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = """
    sphinx.ext.autodoc
    sphinx.ext.autosummary
    sphinx.ext.githubpages
    sphinx.ext.todo
    sphinx.ext.viewcode
    sphinx.ext.imgmath
""".split()

templates_path = ["_templates"]
#source_suffix = ".rst"
exclude_patterns = []

# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

# html_theme = 'alabaster'
# html_theme = 'sphinxdoc'
html_theme = "pydata_sphinx_theme"
html_static_path = ["_static"]
