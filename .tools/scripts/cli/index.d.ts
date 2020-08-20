
export type EnvVars = { [name: string]: string }

export type CommandOptionsType = {
  cwd?: string
  env?: { [name: string]: string }
  ignoreStatus: boolean
}

export const command: {
  exec(command: string, options: CommandOptionsType): number
  call(program: string, args: string[], options: CommandOptionsType): number
  exit(status: number)
}

export const directory: {
  exists(path: string): boolean
  filenames(path: string): string[]
  copy(src: string, dest: string, filter?: (name: string, path: string, stats: fs.Stats) => boolean)
  make(path: string)
  remove(path: string)
  clean(path: string)
}

export const file: {
  exists(path: string): boolean
  remove(path: string)
  copy: {
    toFile(src: string, dest: string): string
    toDir(src: string, dest: string): string
  }
  move: {
    toFile(src: string, dest: string): string
    toDir(src: string, dest: string): string
  }
  read: {
    json(path: string): any
    text(path: string): string
  }
  write: {
    json(path: string, data: any)
    text(path: string, data: string)
  }
}

export const print: {
  log(...args)
  debug(...args)
  warning(...args)
  error(...args)
  success(...args)
  title(...args)
  info(...args)
  exception(exception: Error)
}

export type ScriptArgumentType = {
  type: string
  enums?: string[]
  isArray?: boolean
  required?: boolean
}

export type ScriptDescriptorType = {
  arguments?: {
    [name: string]: ScriptArgumentType
  }
}

export function script(program: (argv: any[]) => void, descriptor: ScriptDescriptorType)

