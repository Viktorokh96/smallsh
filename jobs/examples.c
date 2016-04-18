
/* Обрывки кода которые могут помочь */

/* Return the fd from which we are actually getting input. */
#define input_tty() (shell_tty != -1) ? shell_tty : fileno (stderr)


/* Fill the contents of shell_tty_info with the current tty info. */
get_tty_state ()
{
  int tty = input_tty();

  if (tty != -1)
    {
#if defined (NEW_TTY_DRIVER)
      ioctl (tty, TIOCGETP, &shell_tty_info);
      ioctl (tty, TIOCGETC, &shell_tchars);
      ioctl (tty, TIOCGLTC, &shell_ltchars);
#endif /* NEW_TTY_DRIVER */

#if defined (TERMIO_TTY_DRIVER)
      ioctl (tty, TCGETA, &shell_tty_info);
#endif /* TERMIO_TTY_DRIVER */

#if defined (TERMIOS_TTY_DRIVER)
      if (tcgetattr (tty, &shell_tty_info) < 0)
	{
#if 0
	  /* Only print an error message if we're really interactive at
	     this time. */
	  if (interactive)
	    internal_error ("[%d: %d] tcgetattr: %s",
			    getpid (), shell_level, strerror (errno));
#endif
	  return -1;
	}
#endif /* TERMIOS_TTY_DRIVER */
    }
  return 0;
}

/* Make the current tty use the state in shell_tty_info. */
set_tty_state ()
{
  int tty = input_tty ();

  if (tty != -1)
    {
#if defined (NEW_TTY_DRIVER)
#  if defined (DRAIN_OUTPUT)
      draino (tty, shell_tty_info.sg_ospeed);
#  endif /* DRAIN_OUTPUT */
      ioctl (tty, TIOCSETN, &shell_tty_info);
      ioctl (tty, TIOCSETC, &shell_tchars);
      ioctl (tty, TIOCSLTC, &shell_ltchars);
#endif /* NEW_TTY_DRIVER */

#if defined (TERMIO_TTY_DRIVER)
      ioctl (tty, TCSETAW, &shell_tty_info);
#endif /* TERMIO_TTY_DRIVER */

#if defined (TERMIOS_TTY_DRIVER)
      if (tcsetattr (tty, TCSADRAIN, &shell_tty_info) < 0)
	{
	  /* Only print an error message if we're really interactive at
	     this time. */
	  if (interactive)
	    internal_error ("[%d: %d] tcsetattr: %s",
			    getpid (), shell_level, strerror (errno));
	  return -1;
	}
#endif /* TERMIOS_TTY_DRIVER */
    }
  return 0;
}



static int sigint_blocked = 0;
/* Cause SIGINT to not be delivered until the corresponding call to
   release_sigint(). */

static void
block_sigint ()
{
  if (sigint_blocked)
    return;

#if defined (HAVE_POSIX_SIGNALS)
  sigemptyset (&sigint_set);
  sigemptyset (&sigint_oset);
  sigaddset (&sigint_set, SIGINT);
  sigprocmask (SIG_BLOCK, &sigint_set, &sigint_oset);
#else /* !HAVE_POSIX_SIGNALS */
#  if defined (HAVE_BSD_SIGNALS)
  sigint_oldmask = sigblock (sigmask (SIGINT));
#  else /* !HAVE_BSD_SIGNALS */
#    if defined (HAVE_USG_SIGHOLD)
  sighold (SIGINT);
#    endif /* HAVE_USG_SIGHOLD */
#  endif /* !HAVE_BSD_SIGNALS */
#endif /* !HAVE_POSIX_SIGNALS */
  sigint_blocked = 1;
}

/* Allow SIGINT to be delivered. */
static void
release_sigint ()
{
  if (!sigint_blocked)
    return;

#if defined (HAVE_POSIX_SIGNALS)
  sigprocmask (SIG_SETMASK, &sigint_oset, (sigset_t *)NULL);
#else
#  if defined (HAVE_BSD_SIGNALS)
  sigsetmask (sigint_oldmask);
#  else /* !HAVE_BSD_SIGNALS */
#    if defined (HAVE_USG_SIGHOLD)
  sigrelse (SIGINT);
#    endif /* HAVE_USG_SIGHOLD */
#  endif /* !HAVE_BSD_SIGNALS */
#endif /* !HAVE_POSIX_SIGNALS */

  sigint_blocked = 0;
}

set_tty_settings (tty, tiop)
     int tty;
     TIOTYPE *tiop;
{
  if (tiop->flags & SGTTY_SET)
    {
      ioctl (tty, TIOCSETN, &(tiop->sgttyb));
      tiop->flags &= ~SGTTY_SET;
    }
  readline_echoing_p = 1;

#if defined (TIOCLSET)
  if (tiop->flags & LFLAG_SET)
    {
      ioctl (tty, TIOCLSET, &(tiop->lflag));
      tiop->flags &= ~LFLAG_SET;
    }
#endif

#if defined (TIOCSETC)
  if (tiop->flags & TCHARS_SET)
    {
      ioctl (tty, TIOCSETC, &(tiop->tchars));
      tiop->flags &= ~TCHARS_SET;
    }
#endif

#if defined (TIOCSLTC)
  if (tiop->flags & LTCHARS_SET)
    {
      ioctl (tty, TIOCSLTC, &(tiop->ltchars));
      tiop->flags &= ~LTCHARS_SET;
    }
#endif

  return 0;
}

static int
get_tty_settings (tty, tiop)
     int tty;
     TIOTYPE *tiop;
{
  int ioctl_ret;
#if !defined (SHELL) && defined (TIOCGWINSZ)
  struct winsize w;

  if (ioctl (tty, TIOCGWINSZ, &w) == 0)
      (void) ioctl (tty, TIOCSWINSZ, &w);
#endif

  /* Keep looping if output is being flushed after a ^O (or whatever
     the flush character is). */
  while ((ioctl_ret = GETATTR (tty, tiop)) < 0 || OUTPUT_BEING_FLUSHED (tiop))
    {
      if (ioctl_ret < 0 && errno != EINTR)
  return -1;
      if (OUTPUT_BEING_FLUSHED (tiop))
        continue;
      errno = 0;
    }
  return 0;
}

static int
set_tty_settings (tty, tiop)
     int tty;
     TIOTYPE *tiop;
{
  while (SETATTR (tty, tiop) < 0)
    {
      if (errno != EINTR)
  return -1;
      errno = 0;
    }

#if 0

#if defined (TERMIOS_TTY_DRIVER)
#  if defined (__ksr1__)
  if (ksrflow)
    {
      ksrflow = 0;
      tcflow (tty, TCOON);
    }
#  else /* !ksr1 */
  tcflow (tty, TCOON);    /* Simulate a ^Q. */
#  endif /* !ksr1 */
#else
  ioctl (tty, TCXONC, 1); /* Simulate a ^Q. */
#endif /* !TERMIOS_TTY_DRIVER */

#endif

  return 0;
}

static void
prepare_terminal_settings (meta_flag, otio, tiop)
     int meta_flag;
     TIOTYPE otio, *tiop;
{
  readline_echoing_p = (otio.c_lflag & ECHO);

  tiop->c_lflag &= ~(ICANON | ECHO);

  if ((unsigned char) otio.c_cc[VEOF] != (unsigned char) _POSIX_VDISABLE)
    _rl_eof_char = otio.c_cc[VEOF];

#if defined (USE_XON_XOFF)
#if defined (IXANY)
  tiop->c_iflag &= ~(IXON | IXOFF | IXANY);
#else
  /* `strict' Posix systems do not define IXANY. */
  tiop->c_iflag &= ~(IXON | IXOFF);
#endif /* IXANY */
#endif /* USE_XON_XOFF */

  /* Only turn this off if we are using all 8 bits. */
  if (((tiop->c_cflag & CSIZE) == CS8) || meta_flag)
    tiop->c_iflag &= ~(ISTRIP | INPCK);

  /* Make sure we differentiate between CR and NL on input. */
  tiop->c_iflag &= ~(ICRNL | INLCR);

#if !defined (HANDLE_SIGNALS)
  tiop->c_lflag &= ~ISIG;
#else
  tiop->c_lflag |= ISIG;
#endif

  tiop->c_cc[VMIN] = 1;
  tiop->c_cc[VTIME] = 0;

#if defined (FLUSHO)
  if (OUTPUT_BEING_FLUSHED (tiop))
    {
      tiop->c_lflag &= ~FLUSHO;
      otio.c_lflag &= ~FLUSHO;
    }
#endif

  /* Turn off characters that we need on Posix systems with job control,
     just to be sure.  This includes ^Y and ^V.  This should not really
     be necessary.  */
#if defined (TERMIOS_TTY_DRIVER) && defined (_POSIX_VDISABLE)

#if defined (VLNEXT)
  tiop->c_cc[VLNEXT] = _POSIX_VDISABLE;
#endif

#if defined (VDSUSP)
  tiop->c_cc[VDSUSP] = _POSIX_VDISABLE;
#endif

#endif /* TERMIOS_TTY_DRIVER && _POSIX_VDISABLE */
}
#endif  /* NEW_TTY_DRIVER */


/* Put the terminal in CBREAK mode so that we can detect key presses. */
void
rl_prep_terminal (meta_flag)
     int meta_flag;
{
#if !defined (__GO32__)
  int tty = fileno (rl_instream);
  TIOTYPE tio;

  if (terminal_prepped)
    return;

  /* Try to keep this function from being INTerrupted. */
  block_sigint ();

  if (get_tty_settings (tty, &tio) < 0)
    {
      release_sigint ();
      return;
    }

  otio = tio;

  prepare_terminal_settings (meta_flag, otio, &tio);

  if (set_tty_settings (tty, &tio) < 0)
    {
      release_sigint ();
      return;
    }

  control_meta_key (1);
#if 0
  control_keypad (1);
#endif
  fflush (rl_outstream);
  terminal_prepped = 1;

  release_sigint ();
#endif /* !__GO32__ */
}


rl_stop_output (count, key)
     int count, key;
{
  int fildes = fileno (rl_instream);

#if defined (TIOCSTOP)
# if defined (apollo)
  ioctl (&fildes, TIOCSTOP, 0);
# else
  ioctl (fildes, TIOCSTOP, 0);
# endif /* apollo */
#else /* !TIOCSTOP */
# if defined (TERMIOS_TTY_DRIVER)
#  if defined (__ksr1__)
  ksrflow = 1;
#  endif /* ksr1 */
  tcflow (fildes, TCOOFF);
# else
#   if defined (TCXONC)
  ioctl (fildes, TCXONC, TCOON);
#   endif /* TCXONC */
# endif /* !TERMIOS_TTY_DRIVER */
#endif /* !TIOCSTOP */

  return 0;
}

void
rltty_set_default_bindings (kmap)
     Keymap kmap;
{
  TIOTYPE ttybuff;
  int tty = fileno (rl_instream);

#if defined (NEW_TTY_DRIVER)

#define SET_SPECIAL(sc, func) \
  do \
    { \
      int ic; \
      ic = sc; \
      if (ic != -1 && kmap[ic].type == ISFUNC) \
  kmap[ic].function = func; \
    } \
  while (0)

  if (get_tty_settings (tty, &ttybuff) == 0)
    {
      if (ttybuff.flags & SGTTY_SET)
  {
    SET_SPECIAL (ttybuff.sgttyb.sg_erase, rl_rubout);
    SET_SPECIAL (ttybuff.sgttyb.sg_kill, rl_unix_line_discard);
  }

#  if defined (TIOCGLTC)
      if (ttybuff.flags & LTCHARS_SET)
  {
    SET_SPECIAL (ttybuff.ltchars.t_werasc, rl_unix_word_rubout);
    SET_SPECIAL (ttybuff.ltchars.t_lnextc, rl_quoted_insert);
  }
#  endif /* TIOCGLTC */
    }

#else /* !NEW_TTY_DRIVER */

#define SET_SPECIAL(sc, func) \
  do \
    { \
      unsigned char uc; \
      uc = ttybuff.c_cc[sc]; \
      if (uc != (unsigned char)_POSIX_VDISABLE && kmap[uc].type == ISFUNC) \
  kmap[uc].function = func; \
    } \
  while (0)

  if (get_tty_settings (tty, &ttybuff) == 0)
    {
      SET_SPECIAL (VERASE, rl_rubout);
      SET_SPECIAL (VKILL, rl_unix_line_discard);

#  if defined (VLNEXT) && defined (TERMIOS_TTY_DRIVER)
      SET_SPECIAL (VLNEXT, rl_quoted_insert);
#  endif /* VLNEXT && TERMIOS_TTY_DRIVER */

#  if defined (VWERASE) && defined (TERMIOS_TTY_DRIVER)
      SET_SPECIAL (VWERASE, rl_unix_word_rubout);
#  endif /* VWERASE && TERMIOS_TTY_DRIVER */
    }
#endif /* !NEW_TTY_DRIVER */
}
