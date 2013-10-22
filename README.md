# GNOME Builder

This is a prototype.
There is not much to see yet.
If you would like to help, email me!

## Testing

```sh
./autogen.sh --enable-silent-rules --enable-debug=minimum
make
cd src
sudo make install-gsettings-schemas
make run
```

