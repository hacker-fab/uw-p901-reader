#![no_std]

use core::{
    fmt::{Arguments, Debug, Write},
    slice,
};

use alloc::{
    string::{FromUtf8Error, String},
    vec::Vec,
};
use embedded_io::Error;
use embedded_io_async::Read;

extern crate alloc;


/// Error type returned by [`read_line_dbg`].
pub enum ReadlineError<T: Read> {
    /// An I/O error occured on the underlying stream.
    IO(T::Error),
    /// The received line isn't valid UTF-8.
    Utf8(FromUtf8Error),
}

impl<T: Read> Error for ReadlineError<T> {
    fn kind(&self) -> embedded_io::ErrorKind {
        match self {
            ReadlineError::IO(io) => io.kind(),
            ReadlineError::Utf8(_) => embedded_io::ErrorKind::InvalidData,
        }
    }
}

impl<T: Read> Debug for ReadlineError<T> {
    fn fmt(&self, f: &mut core::fmt::Formatter<'_>) -> core::fmt::Result {
        match self {
            Self::IO(arg0) => f.debug_tuple("IO").field(arg0).finish(),
            Self::Utf8(arg0) => f.debug_tuple("Utf8").field(arg0).finish(),
        }
    }
}

/// Crude function for reading lines from any input stream.
/// This is mainly for reading/writing to the computer serial over UART0.
pub async fn read_line_dbg<T: Read>(src: &mut T) -> Result<String, ReadlineError<T>> {
    let mut next = 0u8;
    let mut buffer = Vec::<u8>::new();
    // Read bits one at a time until either an I/O error happens or we hit a newline.
    while {
        src.read(slice::from_mut(&mut next))
            .await
            .map_err(ReadlineError::IO)?;
        next != b'\r'
    } {
        match next {
            // ignore non-printable characters for now.
            0..31 | 127 => (),
            _ => buffer.push(next)
        }
    }
    // Convert the byte buffer to a string buffer.
    let sbuffer = String::from_utf8(buffer).map_err(ReadlineError::Utf8)?;
    Ok(sbuffer)
}
