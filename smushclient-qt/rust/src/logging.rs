use cxx_qt_lib::{QString, QtMsgType};
use log::LevelFilter;
use simple_logger::SimpleLogger;

pub fn create_logger() -> SimpleLogger {
    #[cfg(debug_assertions)]
    let level = LevelFilter::Info;
    #[cfg(not(debug_assertions))]
    let level = LevelFilter::Error;

    SimpleLogger::new()
        .with_level(level)
        .env()
        .without_timestamps()
        .with_colors(true)
}

#[derive(Copy, Clone)]
pub struct QMessageLogContext<'a> {
    pub msg_type: QtMsgType,
    pub message: &'a QString,
    pub category: Option<&'a str>,
    pub file: Option<&'a str>,
    pub function: Option<&'a str>,
    pub line: i32,
}

impl<'a> QMessageLogContext<'a> {
    pub fn level(&self) -> log::Level {
        match self.msg_type {
            QtMsgType::QtCriticalMsg => log::Level::Error,
            QtMsgType::QtWarningMsg => log::Level::Warn,
            QtMsgType::QtInfoMsg => log::Level::Info,
            QtMsgType::QtDebugMsg => log::Level::Debug,
            _ => log::Level::Trace,
        }
    }

    pub fn target(&self) -> &'a str {
        match self.category {
            Some("default") | None => (),
            Some(category) => return category,
        }
        let Some(function) = self.function else {
            return "";
        };
        let function = match function.split_once(' ') {
            Some((_, function)) => function,
            None => function,
        };
        match function.split_once('(') {
            Some((function, _)) => function,
            None => function,
        }
    }
}

pub fn log(context: QMessageLogContext<'_>) {
    let logger = log::logger();
    let metadata = log::Metadata::builder()
        .target(context.target())
        .level(context.level())
        .build();
    if !logger.enabled(&metadata) {
        return;
    }
    logger.log(
        &log::Record::builder()
            .metadata(metadata)
            .args(format_args!("{}", context.message))
            .file(context.file)
            .line(context.line.try_into().ok())
            .build(),
    );
}
