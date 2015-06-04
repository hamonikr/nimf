uuid = dasom-agent@gnome-shell-extensions.cogno.org

extensiondir = /usr/share/gnome-shell/extensions/$(uuid)

extension_DATA = \
	metadata.json \
	stylesheet.css

extension_SCRIPTS = \
	extension.js

EXTRA_DIST = $(extension_DATA)

uninstall-local:
	-rm -r $(extensiondir)
