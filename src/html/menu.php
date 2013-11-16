<nav class="navbar navbar-default" role="navigation">
        <!-- Brand and toggle get grouped for better mobile display -->
        <div class="navbar-header">
                <button type="button" class="navbar-toggle" data-toggle="collapse" data-target=".navbar-ex1-collapse">
                        <span class="sr-only">Toggle navigation</span>
                        <span class="icon-bar"></span>
                        <span class="icon-bar"></span>
                        <span class="icon-bar"></span>
                </button>
                <a class="navbar-brand" href="index.php">Test CMS Users</a>
        </div>
        <p class="navbar-text pull-right">
        <?php if ( is_Logged() ) :?>
                <a href="logout.php" class="navbar-link">Log out</a>
        <?php endif; ?>
        </p>
</nav>
