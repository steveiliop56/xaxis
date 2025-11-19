# XAxis - Use Xbox controller triggers as rudders in X-Plane

This is an attempt of mine in making a plugin for X-Plane. My triggers appearing as separate axis was the excuse I needed to have some fun. So, I made XAxis which reads the values from the Xbox controller and merges them into one value which it then uses in the simulator yaw control. It also supports storing the configuration in a file and axis polling in order to find your left and right triggers.

> [!WARNING]
> This is the first time I have writtent C++ code _and it is probably terrible_ so, expect bugs.

> [!NOTE]
> This plugin was developed for X-Plane 11 although it should work for 12 too. If you face any issues please let me know.

## Usage

Using the plugin is relatively easy, you just need to follow the steps below:

1. Download the plugin from the [latest release](https://github.com/steveiliop56/xaxis/releases/latest) and place it in the `Resources/plugins` directory. Make sure to keep the `xaxis` name in order for the plugin to be able to read the configuration.

> [!NOTE]
> The plugin is only published for x64 Linux. If you have a different OS, you will need to build the plugin yourself. Windows versions may be published in the future.

2. Create an initial configuration file for XAxis, it should be located in `Resources/plugins/xaxis/config.txt` and contain the following:

```
0,0,32
```

> [!TIP]
> You can change the last value to something bigger if no axis are detected in the next step.

3. Enter in a new flight in X-Plane and under _Plugins/XAxis_ click _Start polling_ and open the debug window. Start clicking your triggers and you should see the axis appearing in the window. You should then have your left and right axis numbers.

4. Modify the config to incude your axis numbers:

```
<value-of-right-axis>,<value-of-left-axis>,32
```

5. Click the _Reload config_ button in the XAxis menu and enjoy!

## Building

If you like, you can build the plugin yourself. You will need `make`, `g++` and `gcc` installed. You will also need to download the [X-Plane SDK](https://developer.x-plane.com/sdk/plugin-sdk-downloads/). I used version `3.0.1` in my builds so as it is compatible with X-Plane 11. Make sure your SDK is named `SDK` in the root of the repository. Finally, build with:

```sh
make
```

If everything works, the plugin should be located in `build/xaxis`.

## Contributing

If you like, you can contribute by creating a [pull request](https://github.com/steveiliop56/xaxis/pulls) or by creating an [issue](https://github.com/steveiliop56/xaxis/issues). Any form of contribution is welcome!

## License

XAaxis is licensed under the MIT License. TL;DR — You can use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the software. Just make sure to include the original license in any substantial portions of the code. There’s no warranty — use at your own risk. See the [LICENSE](./LICENSE) file for full details.
