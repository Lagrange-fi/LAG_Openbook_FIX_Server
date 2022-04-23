use chrono::Utc;

pub trait Logger {
    type Priority : ToString;
    fn log(&self, priority: Self::Priority, message: &str);
}

pub enum LogPriority {
    Trace,
    Debug,
    Info,
    Warning,
    Error,
}

impl ToString for LogPriority {
    fn to_string(&self) -> String {
        match self {
            LogPriority::Trace => String::from("Trace"),
            LogPriority::Debug => String::from("Debug"),
            LogPriority::Info => String::from("Info"),
            LogPriority::Warning => String::from("Warning"),
            LogPriority::Error => String::from("Error")
        } 
    }
}

pub struct ConsoleLogger;

impl<'a> Logger for ConsoleLogger {
    type Priority = LogPriority;
    fn log(&self, priority: Self::Priority, message: &str) {
        println!("{} | {} | {}", Utc::now().to_string(), priority.to_string(), message)
    }
}