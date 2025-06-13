#![no_std]
#![no_main]

use embassy_executor::Spawner;
use embassy_time::{Duration, Timer};
use esp_backtrace as _;
use esp_hal::clock::CpuClock;
use esp_hal::timer::timg::TimerGroup;
use esp_hal::uart::{AtCmdConfig, Config, RxConfig, Uart};
use p901_reader_code::read_line_dbg;
use core::fmt::Write;
use core::slice;

extern crate alloc;

const READ_BUF_SIZE: usize = 64;

#[esp_hal_embassy::main]
async fn main(spawner: Spawner) {
    // generator version: 0.3.1

    // Initialize the ESP stuff.
    let config = esp_hal::Config::default().with_cpu_clock(CpuClock::max());
    let peripherals = esp_hal::init(config);

    // Setup a 72kB heap. This should be big enough for
    // most dynamic things we need to worry about.
    esp_alloc::heap_allocator!(size: 72 * 1024);
    
    // Initialize the async executor.
    let timer0 = TimerGroup::new(peripherals.TIMG1);
    esp_hal_embassy::init(timer0.timer0);

    // Radio init code. Won't need this until later.
    // let timer1 = TimerGroup::new(peripherals.TIMG0);
    // let _init = esp_wifi::init(
    //     timer1.timer0,
    //     esp_hal::rng::Rng::new(peripherals.RNG),
    //     peripherals.RADIO_CLK,
    // )
    // .unwrap();

    // TODO: Embassy lets us execute async tasks in parallel via the spawner.
    let _ = spawner;

    let uart_cfg = Config::default()
        .with_rx(RxConfig::default().with_fifo_full_threshold(READ_BUF_SIZE as u16))
        .with_baudrate(9600);

    // UART0 is the UART connection to the computer
    let mut uart = Uart::new(peripherals.UART0, uart_cfg)
        .unwrap()
        .with_tx(peripherals.GPIO1)
        .with_rx(peripherals.GPIO3)
        .into_async();
    uart.set_at_cmd(AtCmdConfig::default().with_cmd_char(0x04u8));

    loop {
        // This is equivalent to reading/writing from stdin/stdout.
        write!(&mut uart, "input: ").unwrap();
        let line = read_line_dbg(&mut uart).await.unwrap();
        write!(&mut uart, "received: {}\r\n", line).unwrap();
    }
}
