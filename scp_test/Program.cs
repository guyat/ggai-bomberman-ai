using System;
using System.Threading;
using ScpDriverInterface;

try
{
    Console.WriteLine("creating ScpBus...");
    using var bus = new ScpBus();

    Console.WriteLine("ScpBus created.");
    Console.WriteLine("plugging in pad 1...");
    bool plugResult = bus.PlugIn(1);
    Console.WriteLine("PlugIn result: " + plugResult);

    var controller = new X360Controller();

    controller.Buttons = X360Buttons.None;
    bus.Report(1, controller.GetReport());

    Console.WriteLine("open joy.cpl test window now.");
    Console.WriteLine("press Enter to start RIGHT on/off test.");
    Console.ReadLine();

    Console.WriteLine("RIGHT ON (2 sec)...");
    controller.Buttons = X360Buttons.Right;
    bus.Report(1, controller.GetReport());
    Thread.Sleep(2000);

    Console.WriteLine("RIGHT OFF (2 sec)...");
    controller.Buttons = X360Buttons.None;
    bus.Report(1, controller.GetReport());
    Thread.Sleep(2000);

    Console.WriteLine("press Enter to unplug and exit.");
    Console.ReadLine();

    Console.WriteLine("unplugging pad 1...");
    bus.Unplug(1);

    Console.WriteLine("done.");
}
catch (Exception ex)
{
    Console.WriteLine("ERROR:");
    Console.WriteLine(ex.ToString());
}