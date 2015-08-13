uuid = dasom-agent@gnome-shell-extensions.cogno.org

extensiondir = /usr/share/gnome-shell/extensions/$(uuid)

extension_DATA = \
	extension.js \
	metadata.json \
	stylesheet.css

EXTRA_DIST = $(extension_DATA)

uninstall-local:
	-rm -r $(extensiondir)
